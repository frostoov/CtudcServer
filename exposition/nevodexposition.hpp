#pragma once

#include "exposition.hpp"


class NevodExposition : public Exposition {
    using Mutex = std::mutex;
    using Lock = std::lock_guard<Mutex>;
    using EventBuffer = std::vector<Tdc::EventHits>;
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
    };
public:
    NevodExposition(std::shared_ptr<Tdc> tdc,
                    const Settings& settings,
                    const ChannelConfig& config,
                    std::function<void(TrekFreq)> onMonitor = [](auto){ });
    ~NevodExposition();
    operator bool() const override { return mActive; }
    
    void stop() override;
    
    uintmax_t triggerCount() const override { return mTrgCount[0]; }
    uintmax_t triggerDrop() const override { return mTrgCount[1]; }
    
    uintmax_t packageCount() const override { return mPkgCount[0]; }
    uintmax_t packageDrop() const override { return mPkgCount[1]; }
    
    TrekHitCount chambersCount() const override { return mChambersCount[0]; }
    TrekHitCount chambersDrop() const override { return mChambersCount[1]; }
    TimePoint startPoint() const override { return mStartPoint; }
protected:    
    void readLoop(std::shared_ptr<Tdc> tdc, const Settings& settings);
    void writeLoop(const Settings& settings, const ChannelConfig& config);
    void monitorLoop(std::shared_ptr<Tdc> tdc, const Settings& settings, const ChannelConfig& conf);

    std::vector<trek::data::EventHits> handleEvents(const EventBuffer& buffer, const ChannelConfig& conf, bool drop);
    void verifySettings(const Settings& settings);
private:
    EventBuffer mBuffer;
    trek::net::MulticastReceiver mInfoRecv;
    trek::net::MulticastReceiver mCtrlRecv;
    
    std::thread mWriteThread;
    std::thread mMonitorThread;
    std::thread mReadThread;
    
    uintmax_t mTrgCount[2];
    
    uintmax_t mPkgCount[2];
    
    TrekHitCount mChambersCount[2];

    uintmax_t mCurrentGateWidth = 0;
    std::atomic_bool mActive;
    std::function<void(TrekFreq)> mOnMonitor;

    Mutex mBufferMutex;
    Mutex mTdcMutex;
    std::condition_variable mCv;

    TimePoint mStartPoint = Clock::now();
};
