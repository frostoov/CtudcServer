#include "expocontroller.hpp"

#include <gsl/gsl_util.h>

#include <trek/net/request.hpp>
#include <trek/net/response.hpp>
#include <trek/net/controller.hpp>

#include <json.hpp>

using std::make_unique;
using std::string;
using std::ostringstream;
using std::setw;
using std::setfill;
using std::unordered_map;
using std::function;
using std::logic_error;
using std::runtime_error;
using std::chrono::microseconds;

using nlohmann::json;

using trek::Callback;
using trek::net::Request;
using trek::net::Response;
using trek::net::Controller;

template<typename T>
static auto convert(const T& v, const std::string& what) {
    json::array_t jFreq;
    for(auto& chamFreqPair : v) {
        auto& chamFreq = chamFreqPair.second;
        auto  chamNum  = chamFreqPair.first;
        jFreq.push_back({
            {"chamber", chamNum},
            {what,      chamFreq},
        });
    }
    return jFreq;
}

static auto convertFreq(const TrekFreq& freq) {
    return convert(freq, "freq");
}

static auto convertHitCount(const TrekHitCount& count) {
    return convert(count, "count");
}

ExpoController::ExpoController(const std::string& name,
                               const ModulePtr& module,
                               const Exposition::Settings& settings,
                               const ChannelConfig& config)
    : Controller(name, createMethods()),
      mDevice(module),
      mChannelConfig(config),
      mConfig(settings) { }

ExpoController::~ExpoController() {
    
}

const Callback<void(unsigned)>& ExpoController::onNewRun() {
    return mOnNewRun;
}

Controller::Methods ExpoController::createMethods() {
    return {
        {"type",         [&](auto & request, auto & send) { return this->type(request, send); } },
        {"run",          [&](auto & request, auto & send) { return this->run(request, send); } },
        {"launchRead",   [&](auto & request, auto & send) { return this->launchRead(request, send); } },
        {"stopRead",     [&](auto & request, auto & send) { return this->stopRead(request, send); } },
        {"launchFreq",   [&](auto & request, auto & send) { return this->launchFreq(request, send); } },
        {"stopFreq",     [&](auto & request, auto & send) { return this->stopFreq(request, send); } },
        {"triggerCount", [&](auto & request, auto & send) { return this->triggerCount(request, send); } },
        {"packageCount", [&](auto & request, auto & send) { return this->packageCount(request, send); } },
        {"chambersCount",[&](auto & request, auto & send) { return this->chambersCount(request, send); } },
        {"freq",         [&](auto & request, auto & send) { return this->freq(request, send); } },
    };
}

void ExpoController::type(const Request& request, const SendCallback& send) {
    send({ name(), __func__, {getProcessType()} });
}

void ExpoController::run(const Request& request, const SendCallback& send) {
    send({ name(), __func__, {mConfig.nRun} });
}

void ExpoController::launchRead(const Request& request, const SendCallback& send) {
    if(mFreqFuture || mExposition)
        throw logic_error("ExpoController::launchRead process is active");
    assert(!*mExposition);
    mExposition = make_unique<Exposition>(mDevice, mConfig, mChannelConfig, [this](TrekFreq freq) {
        mBroadcast({name(), "freq", {convertFreq(freq)}});
    });
    send({ name(), __func__ });
    handleRequest({ name(), "type"}, mBroadcast);
    handleRequest({ name(), "run"}, mBroadcast);
}

void ExpoController::stopRead(const Request& request, const SendCallback& send) {
    if(!mExposition)
        throw logic_error("ExpoController::stopRead process is not active");
    assert(*mExposition);
    mExposition.reset();
    handleRequest({ name(), "type"}, mBroadcast);
    ++mConfig.nRun;
    mOnNewRun(mConfig.nRun);
    send({ name(), __func__ });
}

void ExpoController::launchFreq(const Request& request, const SendCallback& send) {
    if(mFreqFuture || mExposition)
        throw logic_error("ExpoController::stopRead process is active");
    assert(!*mExposition);
    auto delay = request.inputs.at(0).get<int>();
    if(delay <= 0)
        throw logic_error("ExpoController::launchFreq invalid delay value");
    mFreqFuture = ::launchFreq(mDevice, microseconds(delay));
    send({ name(), __func__ });
    handleRequest({ name(), "type"}, mBroadcast);
}

void ExpoController::stopFreq(const Request& request, const SendCallback& send) {
    if(!mFreqFuture)
        throw logic_error("ExpoController::stopFreq process is not active");
    mFreq = ::convertFreq(mFreqFuture(), mChannelConfig);
    mFreqFuture = nullptr;

    handleRequest({ name(), "type"}, mBroadcast);
    send({ name(), __func__ });
}

void ExpoController::triggerCount(const Request& request, const SendCallback& send) const {
    if(!mExposition)
        throw runtime_error("ExpoController::triggerCount process is not expo");
    assert(*mExposition);
    send({ name(), __func__, {mExposition->triggerCount(), mExposition->triggerDrop()} });
}

void ExpoController::packageCount(const Request& request, const SendCallback& send) const {
    if(!mExposition)
        throw runtime_error("ExpoController::packageCount process is not expo");
    assert(*mExposition);
    send({ name(), __func__, {mExposition->packageCount(), mExposition->packageDrop()} });
}

void ExpoController::chambersCount(const Request& request, const SendCallback& send) const {
    if(!mExposition)
        throw runtime_error("ExpoController::packageCount process is not expo");
    assert(*mExposition);
    auto count = convertHitCount(mExposition->chambersCount());
    auto drop  = convertHitCount(mExposition->chamberDrop());
    send({ name(), __func__, {count, drop} });
}

void ExpoController::freq(const Request& request, const SendCallback& send) const {
    send({ name(), __func__, convertFreq(mFreq) });
}

string ExpoController::getProcessType() const {
    if(mExposition) {
        assert(*mExposition);
        return "expo";
    }
    if(mFreqFuture) {
        assert(!mExposition);
        return "freq";
    }
    assert(!mExposition);
    assert(!mFreqFuture);
    return "idle";
}

