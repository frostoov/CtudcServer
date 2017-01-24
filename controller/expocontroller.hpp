#pragma once

#include "tdc/tdc.hpp"
#include "exposition/channelconfig.hpp"
#include "exposition/nevodexposition.hpp"
#include "exposition/ihepexposition.hpp"
#include "exposition/exposition.hpp"
#include "exposition/freq.hpp"

#include <trek/net/controller.hpp>
#include <trek/common/callback.hpp>

#include <json.hpp>

#include <future>

class ExpoContr : public trek::net::Controller {
    using FreqFuture = std::function<ChannelFreq()>;
    using ProcessPtr = std::unique_ptr<Process>;
    using DevicePtr = std::shared_ptr<Tdc>;
    using DeviceMap = std::unordered_map<std::string, DevicePtr>;
public:
    struct Settings {
        uintmax_t   nRun;
        uintmax_t   eventsPerFile;
        uintmax_t   gateWidth;
        std::string writeDir;
        std::string infoIP;
        uint16_t    infoPort;
        std::string ctrlIP;
        uint16_t    ctrlPort;
        uintmax_t   readFreq;
		bool debug;
        
        nlohmann::json marshal() const;
        void unMarshal(const nlohmann::json& doc);

        operator NevodExposition::Settings() const;
        operator IHEPExposition::Settings() const;
    };
public:
    ExpoContr(const std::string& name,
              const std::vector<DevicePtr>& devices,
              const Settings& settings,
              const ChannelConfig& config);
    ~ExpoContr();
    const trek::Callback<void(unsigned)>& onNewRun();
protected:
    Methods createMethods();
    trek::net::Response type(const trek::net::Request& request);
    trek::net::Response run(const trek::net::Request& request);
    trek::net::Response launchRead(const trek::net::Request& request);
    trek::net::Response stopRead(const trek::net::Request& request);
    trek::net::Response launchFreq(const trek::net::Request& request);
    trek::net::Response stopFreq(const trek::net::Request& request);
    trek::net::Response triggerCount(const trek::net::Request& request) const;
    trek::net::Response packageCount(const trek::net::Request& request) const;
    trek::net::Response droppedCount(const trek::net::Request& request) const;
    trek::net::Response chambersCount(const trek::net::Request& request) const;
    trek::net::Response freq(const trek::net::Request& request) const;

    std::string getProcessType() const;
    static TrekFreq createFreq(TrekFreq hitCount, std::chrono::microseconds dur);
    static DeviceMap createDeviceMap(const std::vector<DevicePtr>& devs);
    std::unique_ptr<Exposition> createExposition(const nlohmann::json::array_t& inputs);
private:
    std::unique_ptr<Exposition> mExposition;
    FreqFuture mFreqFuture;
    DeviceMap mDevices;
    ChannelConfig mChannelConfig;
    mutable TrekFreq mFreq;

    Settings           mConfig;
    trek::Callback<void(unsigned)> mOnNewRun;
};
