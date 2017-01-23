#include "nevodexposition.hpp"

#include "eventwriter.hpp"

#include <trek/common/stringbuilder.hpp>
#include <trek/common/timeprint.hpp>
#include <trek/data/nevod.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <future>

using std::string;
using std::vector;
using std::setfill;
using std::setw;
using std::make_unique;
using std::make_shared;
using std::ofstream;
using std::exception;
using std::shared_ptr;
using std::unique_ptr;
using std::unordered_map;
using std::runtime_error;
using std::chrono::system_clock;
using std::chrono::microseconds;
using std::chrono::seconds;

using mkstr = trek::StringBuilder;
using trek::data::NevodPackage;
using trek::data::EventHits;
using trek::data::HitRecord;
using trek::data::EventRecord;

namespace fs = boost::filesystem;

struct EventID {
    EventID(unsigned r, unsigned e) : nRun(r), nEvent(e) { }
    unsigned nRun;
    unsigned nEvent;
};


static std::string monitoringPath(const std::string& dir, unsigned run, unsigned cham) {
    auto monitDir = fs::path(runPath(dir, run)) / "monitoring";
    if(!fs::exists(monitDir))
        fs::create_directories(monitDir);
    else if(!fs::is_directory(monitDir))
        throw runtime_error("monitoringPath invalid dir");
    string filename = mkstr() << "chamber_" << setw(2) << setfill('0') << (cham+1);
    return (monitDir / filename).string();
}

struct membuf : public std::streambuf {
    membuf(char* p, size_t n) {
        setg(p, p, p + n);
        setp(p, p + n);
    }
};

static auto handleNvdPkg(vector<char>& buffer) {
    membuf tempBuffer(buffer.data(), buffer.size());
    std::istream stream(&tempBuffer);
    stream.exceptions(stream.badbit | stream.failbit);
    NevodPackage nvdPkg;
    trek::deserialize(stream, nvdPkg);
    if(memcmp(nvdPkg.keyword, "TRACK ", sizeof(nvdPkg.keyword)) != 0)
        throw std::runtime_error("NevodExposition::handleNevodPackage invalid package");
    return nvdPkg;
}

static auto handleCtrlPkg(vector<char>& buffer) {
    membuf tempBuffer(buffer.data(), buffer.size());
    std::istream stream(&tempBuffer);
    stream.exceptions(stream.badbit | stream.failbit);
    char keyword[6];
    trek::deserialize(stream, keyword, sizeof(keyword));

    if(memcmp(keyword, "NVDDC", sizeof(keyword)) != 0)
        throw std::runtime_error("NevodExposition::handleNevodPackage invalid package");
    uint8_t command;
    trek::deserialize(stream, command);
    return command;
}



NevodExposition::NevodExposition(shared_ptr<Tdc> tdc,
                       const Settings& settings,
                       const ChannelConfig& config,
                       std::function<void(TrekFreq)> onMonitor)
    : mInfoRecv(settings.infoIP, settings.infoPort),
      mCtrlRecv(settings.ctrlIP, settings.ctrlPort),
      mTrgCount{0, 0},
      mPkgCount{0, 0},
      mActive(true),
      mOnMonitor(onMonitor) {
    verifySettings(settings);
    if(!tdc->isOpen())
        throw std::logic_error("launchExpo tdc is not open");
    tdc->clear();
    printStartMeta(settings.writeDir, settings.nRun, *tdc);

    mReadThread = std::thread([this, tdc, settings] {
        this->readLoop(tdc, settings);
        printEndMeta(settings.writeDir, settings.nRun);
    });
    mWriteThread = std::thread([this, settings, config] {
        this->writeLoop(settings, config);
    });
    mMonitorThread = std::thread([this, tdc, settings, config] {
        this->monitorLoop(tdc, settings, config);
    });
}

NevodExposition::~NevodExposition() {
    stop();
}

void NevodExposition::stop() {
    mActive = false;
    mInfoRecv.stop();
    mCtrlRecv.stop();
    mCv.notify_one();
    mReadThread.join();
    mMonitorThread.join();
    mWriteThread.join();
}

void NevodExposition::readLoop(shared_ptr<Tdc> tdc, const Settings& settings) {
    vector<Tdc::EventHits> buffer;
    while(mActive) {
        std::this_thread::sleep_for(microseconds(3000));
        try {
            Lock lkt(mTdcMutex);
            tdc->readEvents(buffer);
        } catch(exception& e) {
            buffer.clear();
            std::cerr << "ATTENTION!!! nevod read loop failure " << e.what() << std::endl;
        }
        try {
            Lock lk(mBufferMutex);
            std::move(buffer.begin(), buffer.end(), std::back_inserter(mBuffer));
        } catch(exception& e) {
            std::cerr << "ATTENTION!!! nevod read loop failure " << e.what() << std::endl;
        }
    }
}

void NevodExposition::writeLoop(const Settings& settings, const ChannelConfig& config) {
    unique_ptr<EventID> nvdID;
    EventWriter eventWriter(runPath(settings.writeDir, settings.nRun),
                            formatPrefix(settings.nRun),
                            settings.eventsPerFile);
    mInfoRecv.onRecv([&](vector<char>& nvdMsg) {
        try {
            Lock lk(mBufferMutex);
            auto nvdPkg = handleNvdPkg(nvdMsg);
            ++mCurrentGateWidth;
            if(nvdID != nullptr) {
                if(mCurrentGateWidth == settings.gateWidth) {
                    auto drop = !(nvdID->nRun == nvdPkg.numberOfRun &&
                                  nvdPkg.numberOfRecord - nvdID->nEvent == mBuffer.size());
                    auto num = nvdID->nEvent + 1;
                    std::function<void(EventHits&)> writer = [&](EventHits& event) {
                        eventWriter.writeEvent({nvdID->nRun, num++, event});
                    };
                    if(drop) {
                        writer = [&](EventHits& event) { eventWriter.writeDrop({nvdID->nRun, num++, event}); };
                    }
                    auto events = handleEvents(mBuffer, config, drop);
                    std::for_each(events.begin(), events.end(), writer);
                    mBuffer.clear();
                    mCurrentGateWidth = 0;
                    nvdID = make_unique<EventID>(nvdPkg.numberOfRun, nvdPkg.numberOfRecord);
                }
            } else {
                mBuffer.clear();
                mCurrentGateWidth = 0;
                nvdID = make_unique<EventID>(nvdPkg.numberOfRun, nvdPkg.numberOfRecord);
            }
            if(mCurrentGateWidth >= settings.gateWidth) {
                mBuffer.clear();
                mCurrentGateWidth = 0;
                nvdID = nullptr;
                throw std::runtime_error("mCurrentGateWidth >= settings.gateWidth");
            }
        } catch(exception& e) {
            std::cerr << "ATTENTION!!! nevod write loop failure " << e.what() << std::endl;
        }
    });
    mInfoRecv.run();
}

void NevodExposition::monitorLoop(shared_ptr<Tdc> tdc, const Settings& settings, const ChannelConfig& conf) {
    unordered_map<unsigned, ofstream> streams;
    mCtrlRecv.onRecv([&](vector<char>& msg) {
        try {
            auto command = handleCtrlPkg(msg);
            if(command == 6) {
                auto now = std::chrono::system_clock::now();
                Lock lkt(mTdcMutex);
                Lock lk(mBufferMutex);
                auto prevMode = tdc->mode();
                tdc->setMode(Tdc::Mode::continuous);
                //For conditional variable
                std::mutex m;
                std::unique_lock<std::mutex> l(m);
                auto stop = launchFreq(tdc, microseconds(100));
                mCv.wait_for(l, seconds(50));
                auto freq = stop();
                tdc->setMode(prevMode);
                auto trekFreq = convertFreq(freq, conf);

                for(auto& chamFreq : trekFreq) {
                    auto filename = monitoringPath(settings.writeDir, settings.nRun, chamFreq.first);
                    ofstream stream;
                    stream.exceptions(stream.badbit | stream.failbit);
                    stream.open(filename, stream.binary | stream.app);

                    auto t = std::chrono::system_clock::to_time_t(now);
                    stream << std::put_time(std::gmtime(&t), "%H:%M:%S %d-%m-%Y");
                    for(auto& wireFreq : chamFreq.second)
                        stream << ' ' << wireFreq;
                    stream << '\n';
                }
                mOnMonitor(trekFreq);
            }
        } catch(std::exception& e) {
            std::cerr << "ATTENTION!!! nevod monitor loop failure " << e.what() << std::endl;
        }
    });
    mCtrlRecv.run();
}

vector<EventHits> NevodExposition::handleEvents(const EventBuffer& buffer, const ChannelConfig& conf, bool drop) {
    auto events = convertEvents(buffer, conf);
    auto i = drop ? 1 : 0;
    mTrgCount[i] += events.size();
    mPkgCount[i] += 1;
    for(auto& event : events) {
        for(auto& hit : event) {
            if(mChambersCount[i].count(hit.chamber()) == 0)
               mChambersCount[i].emplace(hit.chamber(), ChamberHitCount{{0, 0, 0, 0}});
            ++mChambersCount[i].at(hit.chamber()).at(hit.wire());
        }
    }
    return events;
}

void NevodExposition::verifySettings(const NevodExposition::Settings &settings) {
    if(settings.gateWidth == 0) {
        throw std::runtime_error("NevodExposition invalid gate Width");
    }
    if(settings.eventsPerFile == 0) {
        throw std::runtime_error("NevodExposition invalid event number per faile");
    }
}

