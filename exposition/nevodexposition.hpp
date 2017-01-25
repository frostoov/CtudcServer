#pragma once

#include "exposition.hpp"
#include "matcher.hpp"
#include "stats.hpp"

#include <fstream>

class NevodExposition : public Exposition {
    using Mutex = std::mutex;
    using Lock = std::lock_guard<Mutex>;
public:
    struct Settings {
        uintmax_t nRun;
        uintmax_t eventsPerFile;
        uintmax_t gateWidth;
        std::string writeDir;
        std::string infoIP;
        uint16_t    infoPort;
        std::string ctrlIP;
        uint16_t    ctrlPort;
		bool debug;
    };
public:
    NevodExposition(std::shared_ptr<Tdc> tdc,
                    const Settings& settings,
                    const ChannelConfig& config,
                    std::function<void(TrekFreq)> onMonitor = [](auto){ });
    ~NevodExposition();
    operator bool() const override { return mActive; }
    
    void stop() override;
    
    uintmax_t triggerCount() const override { return mStats.triggerCount(); }
    uintmax_t triggerDrop() const override { return mStats.triggerDrops(); }
    
    uintmax_t packageCount() const override { return mStats.packageCount(); }
    uintmax_t packageDrop() const override { return mStats.packageDrops(); }
    
    TrekHitCount chambersCount() const override { return mStats.chambersCount(); }
    TrekHitCount chambersDrop() const override { return mStats.chambersDrops(); }
    TimePoint startPoint() const override { return mStartPoint; }
protected:    
    void readLoop(std::shared_ptr<Tdc> tdc, const Settings& settings);
    void writeLoop(const Settings& settings);
    void monitorLoop(std::shared_ptr<Tdc> tdc, const Settings& settings, const ChannelConfig& conf);

    void verifySettings(const Settings& settings);
    ChannelFreq measureFrequency(std::shared_ptr<Tdc> tdc);
private:
    std::vector<Tdc::EventHits> mBuffer;
	EventMatcher mMatcher;
	Statistics mStats;
    trek::net::MulticastReceiver mInfoRecv;
    trek::net::MulticastReceiver mCtrlRecv;
    
    std::thread mWriteThread;
    std::thread mMonitorThread;
    std::thread mReadThread;

	std::ofstream mDebugStream;

    std::atomic_bool mActive;
    std::function<void(TrekFreq)> mOnMonitor;

    Mutex mBufferMutex;
    Mutex mTdcMutex;
    std::condition_variable mCv;

    TimePoint mStartPoint = Clock::now();
};
