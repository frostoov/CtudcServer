#include "exposition.hpp"
#include "freq.hpp"
#include "eventwriter.hpp"

#include "net/packagereceiver.hpp"

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

static auto formatDir(const Exposition::Settings& settings) {
    string dir = StringBuilder() << settings.writeDir << "/run_" << setw(5) << setfill('0') << settings.nRun;
    if(!fs::exists(dir))
        fs::create_directories(dir);
    else if(!fs::is_directory(dir))
        throw std::runtime_error("ProcessController::fromWriteDir invalid dir");
    return dir;
}

static auto formatPrefix(const Exposition::Settings& settings) {
    return string( StringBuilder() << "ctudc_" << setw(5) << setfill('0') << settings.nRun << '_' );
}

static auto printStartMeta(const Exposition::Settings& settings, Tdc& module) {
    std::ofstream stream;
    stream.exceptions(stream.failbit | stream.badbit);
    auto filename = formatDir(settings) + "/meta";
    stream.open(filename, stream.binary | stream.trunc);
    stream << "Run: " << settings.nRun << '\n';
    stream << "Time: " << system_clock::now() << '\n';
    stream << "TDC: " << module.name() << '\n';
    stream << module.settings();
    return filename;
}

static auto printEndMeta(const string& filename) {
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
        throw std::runtime_error("Exposition::handleNevodPackage invalid package");
    return nvdPkg;
}

static auto handleCtrlPkg(vector<char>& buffer) {
    membuf tempBuffer(buffer.data(), buffer.size());
    std::istream stream(&tempBuffer);
    stream.exceptions(stream.badbit | stream.failbit);
    char keyword[6];
    trek::deserialize(stream, keyword, sizeof(keyword));
    
    if(memcmp(keyword, "NVDDC", sizeof(keyword)) != 0)
        throw std::runtime_error("Exposition::handleNevodPackage invalid package");
    uint8_t command;
    trek::deserialize(stream, command);
    return command;
}

static auto convertEdgeDetection(Tdc::EdgeDetection ed) {
    switch(ed) {
    case Tdc::EdgeDetection::leading:
        return HitRecord::Type::leading;
    case Tdc::EdgeDetection::trailing:
        return HitRecord::Type::trailing;
    default:
        throw std::logic_error("EventWriter::convertEdgeDetection invalid value");
    }
}

static auto convertEvents(const vector<Tdc::EventHits>& events, const ChannelConfig& conf) {
    vector<EventHits> newEvents;
    std::transform(events.begin(), events.end(), std::back_inserter(newEvents), [&](auto& eventHits){
        return convertEventHits(eventHits, conf);
    });
    return newEvents;  
}

static auto convertEventHits(const Tdc::EventHits& hits, const ChannelConfig& conf) {
    EventHits newHits;
    std::transform(hits.begin(), hits.end(), std::back_inserter(newHits), [&](auto& hit){
        return convertHit(hit, conf);
    });
    return newHits;      
}

static auto convertHit(const Tdc::Hit& hit, const ChannelConfig& conf) {
    auto& c = conf.at(hit.channel);    
    return HitRecord(convertEdgeDetection(hit.type), c.wire,  c.chamber, hit.time);
}

Exposition::Exposition(shared_ptr<Tdc> tdc,
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

    mReadThread = std::thread(&Exposition::readLoop, this, tdc, std::ref(settings), std::ref(config));
}

Exposition::~Exposition() {
    stop();
    mReadThread.join();
}

void Exposition::readLoop(shared_ptr<Tdc> tdc, const Settings& settings, const ChannelConfig& config) {
    auto metaFilename = printStartMeta(settings, *tdc);
    vector<Tdc::EventHits> buffer;
	unsigned num = 0;
    EventWriter eventWriter(formatDir(settings), formatPrefix(settings), settings.eventsPerFile);
    std::function<void(EventHits&)> writer = [&](EventHits& event) {
		eventWriter.writeEvent({settings.nRun, num++, event});
	};
	while(mActive) {
        std::this_thread::sleep_for(seconds(1));
        tdc->readEvents(buffer);
		auto events = handleEvents(buffer, config, false);
		std::for_each(events.begin(), events.end(), writer);
    }
    printEndMeta(metaFilename);
}

void Exposition::writeLoop(const Settings& settings, const ChannelConfig& config) {
    unique_ptr<EventID> nvdID;
    EventWriter eventWriter(formatDir(settings), formatPrefix(settings), settings.eventsPerFile);

    mInfoRecv.onRecv([this, &eventWriter, &nvdID, &config](vector<char>& nvdMsg) {
        Lock lk(mBufferMutex);
        try {
            auto nvdPkg = handleNvdPkg(nvdMsg);
            if(nvdID) {
                auto drop = !(nvdID && nvdID->nRun == nvdPkg.numberOfRun && nvdPkg.numberOfRecord - nvdID->nEvent == mBuffer.size());
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
        } catch(std::exception& e) {
            std::cerr << "Expo write loop " << e.what() << std::endl;
        }
    });
    mInfoRecv.start();
}

void Exposition::monitorLoop(shared_ptr<Tdc> tdc, const ChannelConfig& conf) {
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
            std::cerr << "Expo monitor loop " << e.what() << std::endl;
        }
    });
    mCtrlRecv.start();
}

vector<EventHits> Exposition::handleEvents(const EventBuffer& buffer, const ChannelConfig& conf, bool drop) {
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
    
void Exposition::Settings::unMarshal(const json& doc) {
    nRun = doc.at("number_of_run");
    eventsPerFile = doc.at("events_per_file");
    writeDir = doc.at("write_dir").get<string>();
    infoIP = doc.at("info_pkg_ip").get<string>();
    infoPort = doc.at("info_pkg_port");
    ctrlIP = doc.at("ctrl_pkg_ip");
    ctrlPort = doc.at("ctrl_pkg_port");
}

json Exposition::Settings::marshal() const {
    return {
        {"number_of_run", nRun},
        {"events_per_file", eventsPerFile},
        {"write_dir", writeDir},
        {"info_pkg_ip", infoIP},
        {"info_pkg_port", infoPort},
        {"ctrl_pkg_ip", ctrlIP},
        {"ctrl_pkg_port", ctrlPort},
    };
}
