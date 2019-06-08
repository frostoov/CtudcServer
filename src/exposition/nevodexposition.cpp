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

const char *recordType[] = {"drop", "good"};

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
    : mMatcher(config),
      mCtrlRecv(settings.ctrlIP, settings.ctrlPort),
      mActive(true),
      mOnMonitor(onMonitor) {
    verifySettings(settings);
    if(!tdc->isOpen())
        throw std::logic_error("launchExpo tdc is not open");
    tdc->clear();
    printStartMeta(settings.writeDir, settings.nRun, *tdc);

	if(settings.debug) {
		mDebugStream.exceptions(ofstream::failbit | ofstream::badbit);
		mDebugStream.open("expo_debug.log", ofstream::binary | ofstream::app);
	}

    mWriteThread = std::thread([this, tdc, settings, config] {
        this->writeLoop(tdc, settings, config);
        printEndMeta(settings.writeDir, settings.nRun);
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
    mCtrlRecv.stop();
    mCv.notify_one();
    mMonitorThread.join();
    mWriteThread.join();
}

void NevodExposition::writeLoop(shared_ptr<Tdc> tdc, const Settings& settings, const ChannelConfig& config) {
    vector<Tdc::EventHits> buffer;
    vector<trek::data::EventRecord> records;
    EventWriter eventWriter(runPath(settings.writeDir, settings.nRun),
                            formatPrefix(settings.nRun),
                            settings.eventsPerFile);
    const auto matched = true;
    uint64_t nEvent = 0;
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
            records.clear();
            for (const auto& event : convertEvents(buffer, config)) {
                records.emplace_back(settings.nRun, nEvent++, event);
                eventWriter.write(records.back(), matched);
            }
            mStats.incrementTriggers(records.size(), matched);
            mStats.incrementChambersCount(records, matched);
        } catch(exception& e) {
            std::cerr << "ATTENTION!!! nevod write loop failure " << e.what() << std::endl;
        }
    }
}
void NevodExposition::monitorLoop(shared_ptr<Tdc> tdc, const Settings& settings, const ChannelConfig& conf) {
    unordered_map<unsigned, ofstream> streams;
    mCtrlRecv.onRecv([&](vector<char>& msg) {
        try {
            auto command = handleCtrlPkg(msg);
			if(command != 6) {
				return;
			}
        } catch(std::exception e) {
            std::cerr << "Failed parse nevod ctrl message: " << e.what() << std::endl;
        }

        try {
			auto now = std::chrono::system_clock::now();

            Lock lkt(mTdcMutex);
            mMatcher = EventMatcher(conf);

            auto freq = measureFrequency(tdc);
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
        } catch(std::exception& e) {
            std::cerr << "Monitoring failed: " << e.what() << std::endl;
        }
    });
    mCtrlRecv.run();
}

void NevodExposition::verifySettings(const NevodExposition::Settings &settings) {
    if(settings.eventsPerFile == 0) {
        throw std::runtime_error("NevodExposition invalid event number per faile");
    }
}

ChannelFreq NevodExposition::measureFrequency(shared_ptr<Tdc> tdc) {
    auto prevMode = tdc->mode();
    tdc->setMode(Tdc::Mode::continuous);
    //For conditional variable
    std::mutex m;
    std::unique_lock<std::mutex> l(m);
    auto stop = launchFreq(tdc, microseconds(100));
    mCv.wait_for(l, seconds(50));
    auto freq = stop();
    tdc->setMode(prevMode);
    tdc->clear();
    return freq;
}
