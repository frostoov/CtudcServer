#pragma once

#include "tdc/tdc.hpp"

#include "channelconfig.hpp"
#include "freq.hpp"

#include <trek/data/eventrecord.hpp>
#include <trek/net/multicastreceiver.hpp>

#include <condition_variable>
#include <atomic>
#include <thread>

class Exposition {
public:
    virtual ~Exposition() { }
    virtual operator bool() const = 0;
    virtual void stop() = 0;
    virtual uintmax_t triggerCount() const = 0;
    virtual uintmax_t triggerDrop() const = 0;
    virtual uintmax_t packageCount() const = 0;
    virtual uintmax_t packageDrop() const = 0;

    virtual TrekHitCount chambersCount() const = 0;
    virtual TrekHitCount chambersDrop() const = 0;
};

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
};

class IHEPExposition : public Exposition {
    using EventBuffer = std::vector<Tdc::EventHits>;
public:
    struct Settings {
        uintmax_t nRun;
        uintmax_t eventsPerFile;
        std::string writeDir;

        uintmax_t readFreq;
    };
public:
    IHEPExposition(std::shared_ptr<Tdc> tdc,
                   const Settings& settings,
                   const ChannelConfig& config);
    ~IHEPExposition();
    operator bool() const override { return mActive; }
    
    void stop() override;
    
    uintmax_t triggerCount() const override { return mTrgCount; }
    uintmax_t triggerDrop() const override { return 0; }
    
    uintmax_t packageCount() const override { return 0; }
    uintmax_t packageDrop() const override { return 0; }
    
    TrekHitCount chambersCount() const override { return mChambersCount; }
    TrekHitCount chambersDrop() const override { return TrekHitCount(); }
protected:
    void readLoop(std::shared_ptr<Tdc> tdc, const Settings& settings, const ChannelConfig chanConf);
    std::vector<trek::data::EventHits> handleEvents(const EventBuffer& buffer, const ChannelConfig& conf);
private:
    std::thread mReadThread;
    
    uintmax_t mTrgCount;
    TrekHitCount mChambersCount;
    
    std::atomic_bool mActive;
    std::condition_variable mCv;
};


TrekFreq convertFreq(const ChannelFreq& freq, const ChannelConfig& conf);
