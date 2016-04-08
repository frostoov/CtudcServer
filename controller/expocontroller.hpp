#pragma once

#include <trek/net/controller.hpp>
#include <trek/common/callback.hpp>
#include <future>

#include "tdc/caenv2718.hpp"
#include "exposition/channelconfig.hpp"
#include "exposition/process.hpp"
#include "exposition/exposition.hpp"
#include "exposition/freq.hpp"

class ExpoController : public trek::net::Controller {
    using FreqFuture = std::function<ChannelFreq()>;
    using ProcessPtr = std::unique_ptr<Process>;
    using ModulePtr  = std::shared_ptr<CaenV2718>;
public:
    ExpoController(const std::string& name,
                   const ModulePtr& module,
                   const Exposition::Settings& settings,
                   const ChannelConfig& config);
    ~ExpoController();
    const trek::Callback<void(unsigned)>& onNewRun();
protected:
    Methods createMethods();
    void type(const trek::net::Request& request, const SendCallback& send);
    void run(const trek::net::Request& request, const SendCallback& send);
    void launchRead(const trek::net::Request& request, const SendCallback& send);
    void stopRead(const trek::net::Request& request, const SendCallback& send);
    void launchFreq(const trek::net::Request& request, const SendCallback& send);
    void stopFreq(const trek::net::Request& request, const SendCallback& send);
    void triggerCount(const trek::net::Request& request, const SendCallback& send) const;
    void packageCount(const trek::net::Request& request, const SendCallback& send) const;
    void droppedCount(const trek::net::Request& request, const SendCallback& send) const;
    void chambersCount(const trek::net::Request& request, const SendCallback& send) const;
    void freq(const trek::net::Request& request, const SendCallback& send) const;

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
