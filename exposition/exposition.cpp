#include "exposition.hpp"
#include "freq.hpp"
#include "eventwriter.hpp"

#include <trek/common/stringbuilder.hpp>
#include <trek/common/timeprint.hpp>
#include <trek/data/nevod.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <chrono>
#include <iomanip>
#include <future>

using std::string;
using std::vector;
using std::setfill;
using std::setw;
using std::make_unique;
using std::make_shared;
using std::exception;
using std::shared_ptr;
using std::unique_ptr;
using std::chrono::system_clock;
using std::chrono::microseconds;
using std::chrono::seconds;

using trek::StringBuilder;
using trek::data::NevodPackage;
using trek::data::EventHits;
using trek::data::HitRecord;
using trek::data::EventRecord;

using nlohmann::json;

namespace fs = boost::filesystem;

struct EventID {
    EventID(unsigned r, unsigned e) : nRun(r), nEvent(e) { }
    unsigned nRun;
    unsigned nEvent;
};

static string formatDir(const string& dir, unsigned run) {
    string runDir = StringBuilder() << dir << "/run_" << setw(5) << setfill('0') << run;
    if(!fs::exists(runDir))
        fs::create_directories(runDir);
    else if(!fs::is_directory(runDir))
        throw std::runtime_error("ProcessController::fromWriteDir invalid dir");
    return runDir;
}

static string formatPrefix(unsigned run) {
    return StringBuilder() << "ctudc_" << setw(5) << setfill('0') << run << '_';
}

static string formatMetaFilename(const string& dir, unsigned run) {
    return formatDir(dir, run) + "/meta";
}

static void printStartMeta(const string& dir, unsigned run, Tdc& module) {
    auto filename = formatMetaFilename(dir, run);
    std::ofstream stream;
    stream.exceptions(stream.failbit | stream.badbit);
    stream.open(filename, stream.binary | stream.trunc);
    stream << "Run: " << run << '\n';
    stream << "Time: " << system_clock::now() << '\n';
    stream << "TDC: " << module.name() << '\n';
    stream << module.settings();
}

static void printEndMeta(const string& dir, unsigned run) {
    auto filename = formatMetaFilename(dir, run);
    std::ofstream stream;
    stream.exceptions(stream.failbit | stream.badbit);
    stream.open(filename, stream.binary | stream.app);
    stream << "Stopped: " << system_clock::now();
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
    mMonitorThread = std::thread([this, tdc, config] {
        this->monitorLoop(tdc, config);
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
    EventWriter eventWriter(formatDir(settings.writeDir, settings.nRun),
                            formatPrefix(settings.nRun),
                            settings.eventsPerFile);

    mInfoRecv.onRecv([this, &eventWriter, &nvdID, &config](vector<char>& nvdMsg) {
        try {
            Lock lk(mBufferMutex);
            auto nvdPkg = handleNvdPkg(nvdMsg);
            if(nvdID) {
                auto drop = !(nvdID && nvdID->nRun == nvdPkg.numberOfRun &&
                              nvdPkg.numberOfRecord - nvdID->nEvent == mBuffer.size());
                auto num = nvdID->nEvent + 1;
                std::function<void(EventHits&)> writer = [&](EventHits& event) {
                    eventWriter.writeEvent({nvdID->nRun, num++, event});
                };
                if(drop) writer = [&](EventHits& event) { eventWriter.writeDrop({nvdID->nRun, num++, event}); };

                auto events = handleEvents(mBuffer, config, drop);
                std::for_each(events.begin(), events.end(), writer);
            }
            mBuffer.clear();
            nvdID = make_unique<EventID>(nvdPkg.numberOfRun, nvdPkg.numberOfRecord);
        } catch(exception& e) {
            std::cerr << "ATTENTION!!! nevod write loop failure " << e.what() << std::endl;
        }
    });
    mInfoRecv.run();
}

void NevodExposition::monitorLoop(shared_ptr<Tdc> tdc, const ChannelConfig& conf) {
    mCtrlRecv.onRecv([this, tdc, conf](vector<char>& msg) {
        try {
            auto command = handleCtrlPkg(msg);
            if(command == 6) {
                std::cerr << std::chrono::system_clock::now() << "Monitoring" << std::endl;
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
                mOnMonitor(convertFreq(freq, conf));
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

IHEPExposition::IHEPExposition(shared_ptr<Tdc> tdc,
                               const Settings& settings,
                               const ChannelConfig& config)
    : mTrgCount(0),
      mActive(true) {
    if(!tdc->isOpen())
        throw std::logic_error("launchExpo tdc is not open");
    tdc->clear();
    mReadThread = std::thread([this, tdc, settings, config]{
        printStartMeta(settings.writeDir, settings.nRun, *tdc);
        this->readLoop(tdc, settings, config);
        printEndMeta(settings.writeDir, settings.nRun);
    });
}

IHEPExposition::~IHEPExposition() {
    stop();
}

void IHEPExposition::stop() {
    mActive = false;
    mReadThread.join();
}

void IHEPExposition::readLoop(shared_ptr<Tdc> tdc, const Settings& settings, const ChannelConfig chanConf) {
    vector<Tdc::EventHits> buffer;
    EventWriter eventWriter(formatDir(settings.writeDir, settings.nRun),
                            formatPrefix(settings.nRun),
                            settings.eventsPerFile);
    unsigned num = 0;
    while(mActive) {
        try {
            std::this_thread::sleep_for(microseconds(settings.readFreq));
            tdc->readEvents(buffer);
            for(auto& e : handleEvents(buffer, chanConf)) {
                eventWriter.writeEvent({settings.nRun, num++, e});
            }
        } catch(exception& e) {
            std::cerr << "ATTENTION!!! ihep read loop failure " << e.what() << std::endl;
        }
    }
}

vector<EventHits> IHEPExposition::handleEvents(const EventBuffer& buffer, const ChannelConfig& conf) {
    auto events = convertEvents(buffer, conf);
    mTrgCount += events.size();
    for(auto& event : events) {
        for(auto& hit : event) {
            if(mChambersCount.count(hit.chamber()) == 0)
               mChambersCount.emplace(hit.chamber(), ChamberHitCount{{0, 0, 0, 0}});
            ++mChambersCount.at(hit.chamber()).at(hit.wire());
        }
    }
    return events;
}




TrekFreq convertFreq(const ChannelFreq& freq, const ChannelConfig& conf) {
    TrekFreq newFreq;
    for(auto& chanFreq : freq) {
        auto& c = conf.at(chanFreq.first);
        if(newFreq.count(c.chamber) == 0)
            newFreq.emplace(c.chamber, ChamberFreq{{0, 0, 0, 0}});
        newFreq.at(c.chamber).at(c.wire) = chanFreq.second;
    }
    return newFreq;
}
