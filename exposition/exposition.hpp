#pragma once

#include "tdc/tdc.hpp"
#include "net/packagereceiver.hpp"

#include "channelconfig.hpp"
#include "freq.hpp"

#include <trek/data/eventrecord.hpp>
#include <json.hpp>

#include <condition_variable>
#include <atomic>
#include <thread>

class Exposition {
    using Mutex = std::mutex;
    using Lock = std::lock_guard<Mutex>;
    using EventBuffer = std::vector<Tdc::EventHits>;
public:
    struct Settings {
        unsigned    nRun;
        unsigned    eventsPerFile;
        std::string writeDir;
        std::string infoIP;
        uint16_t    infoPort;
        std::string ctrlIP;
        uint16_t    ctrlPort;

        nlohmann::json marshal() const;
        void unMarshal(const nlohmann::json& doc);
    };
public:
    Exposition(std::shared_ptr<Tdc> tdc,
               const Settings& settings,
               const ChannelConfig& config,
               std::function<void(TrekFreq)> onMonitor);
    ~Exposition();
    operator bool() const { return mActive; }
    
    void stop() { mActive = false; }
    
    uintmax_t triggerCount() const { return mTrgCount[0]; }
    uintmax_t triggerDrop() const { return mTrgCount[1]; }
    
    uintmax_t packageCount() const { return mPkgCount[0]; }
    uintmax_t packageDrop() const { return mPkgCount[1]; }
    
    TrekHitCount chambersCount() const { return mChambersCount[0]; }
    TrekHitCount chamberDrop() const { return mChambersCount[1]; }
protected:    
    void readLoop(std::shared_ptr<Tdc> tdc, const Settings& settings);
    void writeLoop(const Settings& settings, const ChannelConfig& config);
    void monitorLoop(std::shared_ptr<Tdc> tdc, const ChannelConfig& conf);

    std::vector<trek::data::EventHits> handleEvents(const EventBuffer& buffer, const ChannelConfig& conf, bool drop);
private:
    EventBuffer mBuffer;
    PackageReceiver mInfoRecv;
    PackageReceiver mCtrlRecv;
    
    std::thread mWriteThread;
    std::thread mMonitorThread;
    std::thread mReadThread;
    
    uintmax_t mTrgCount[2];
    
    uintmax_t mPkgCount[2];
    
    TrekHitCount mChambersCount[2];
    
    std::atomic_bool mActive;
    std::function<void(TrekFreq)> mOnMonitor;

    Mutex mBufferMutex;
    Mutex mTdcMutex;
    std::condition_variable mCv;
};


TrekFreq convertFreq(const ChannelFreq& freq, const ChannelConfig& conf);
