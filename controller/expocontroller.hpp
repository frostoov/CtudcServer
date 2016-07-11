#pragma once

#include <trek/net/controller.hpp>
#include <trek/common/callback.hpp>
#include <future>

#include "tdc/tdc.hpp"
#include "exposition/channelconfig.hpp"
#include "exposition/process.hpp"
#include "exposition/exposition.hpp"
#include "exposition/freq.hpp"

class ExpoContr : public trek::net::Controller {
    using FreqFuture = std::function<ChannelFreq()>;
    using ProcessPtr = std::unique_ptr<Process>;
    using ModulePtr  = std::shared_ptr<Tdc>;
public:
    ExpoContr(const std::string& name,
                   const ModulePtr& module,
                   const Exposition::Settings& settings,
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
private:
    std::unique_ptr<Exposition> mExposition;
    FreqFuture mFreqFuture;
    ModulePtr     mDevice;
    ChannelConfig mChannelConfig;
    mutable TrekFreq mFreq;

    Exposition::Settings           mConfig;
    trek::Callback<void(unsigned)> mOnNewRun;
};
