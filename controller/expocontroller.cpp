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

ExpoController::ExpoController(const std::string& name,
                                     const ModulePtr& module,
                                     const Exposition::Settings& settings,
                                     const ChannelConfig& config)
	: Controller(name, createMethods()),
	  mDevice(module),
	  mChannelConfig(config),
	  mConfig(settings) { }

ExpoController::~ExpoController() {
    if(mProcess != nullptr)
        mProcess->stop();
    if(mFuture.valid())
        mFuture.get();
}

const Callback<void(unsigned)>& ExpoController::onNewRun() {
	return mOnNewRun;
}

Controller::Methods ExpoController::createMethods() {
	return {
        {"type",         [&](auto& request, auto& send) { return this->type(request, send); } },
        {"run",          [&](auto& request, auto& send) { return this->run(request, send); } },
        {"startRead",    [&](auto& request, auto& send) { return this->startRead(request, send); } },
        {"stopRead",     [&](auto& request, auto& send) { return this->stopRead(request, send); } },
        {"startFreq",    [&](auto& request, auto& send) { return this->startFreq(request, send); } },
        {"stopFreq",     [&](auto& request, auto& send) { return this->stopFreq(request, send); } },
        {"triggerCount", [&](auto& request, auto& send) { return this->triggerCount(request, send); } },
        {"packageCount", [&](auto& request, auto& send) { return this->packageCount(request, send); } },
        {"duration",     [&](auto& request, auto& send) { return this->duration(request, send); } },
        {"freq",         [&](auto& request, auto& send) { return this->freq(request, send); } },
	};
}

void ExpoController::type(const Request& request, const SendCallback& send) {
	send({ name(), __func__, {getProcessType()} });
}

void ExpoController::run(const Request& request, const SendCallback& send) {
	send({ name(), __func__, {mConfig.nRun} });
}

void ExpoController::startRead(const Request& request, const SendCallback& send) {
	if(mFuture.valid())
		throw logic_error("ExpoController::startRead process is active");
	mProcess = make_unique<Exposition>(mDevice, mConfig, mChannelConfig);
	mFuture = std::async(std::launch::async, [&] {mProcess->run();});
	send({ name(), __func__ });
    handleRequest({ name(), "type"}, mBroadcast);
    handleRequest({ name(), "run"}, mBroadcast);
}

void ExpoController::stopRead(const Request& request, const SendCallback& send) {
    if(!mFuture.valid())
        throw logic_error("ExpoController::stopRead process is not active");
	auto readManager = dynamic_cast<Exposition*>(mProcess.get());
	if(readManager == nullptr)
        throw logic_error("ExpoController::stopRead process already active");
    readManager->stop();
    auto f = gsl::finally([&]{
        mProcess.reset();
        handleRequest({ name(), "type"}, mBroadcast);
    });
    mFuture.get();
	++mConfig.nRun;
	mOnNewRun(mConfig.nRun);
	send({ name(), __func__ });
}

void ExpoController::startFreq(const Request& request, const SendCallback& send) {
	if(mFuture.valid())
		throw logic_error("ExpoController::stopRead process is active");
	auto delay = request.inputs.at(0).get<int>();
	if(delay <= 0)
		throw logic_error("ExpoController::startFreq invalid delay value");
	mProcess = make_unique<FreqHandler>(mDevice, mChannelConfig, microseconds(delay));
	mFuture = std::async(std::launch::async, [&] {mProcess->run();});
	send({ name(), __func__ });
    handleRequest({ name(), "type"}, mBroadcast);
}

void ExpoController::stopFreq(const Request& request, const SendCallback& send) {
    if(!mFuture.valid())
        throw logic_error("ExpoController::stopFreq process is not active");
	auto freqHandler = dynamic_cast<FreqHandler*>(mProcess.get());
	if(freqHandler == nullptr || !mFuture.valid())
		throw logic_error("ExpoController::stopFreq process is not freq");
	freqHandler->stop();
    auto f = gsl::finally([&]{
        mFreq = createFreq(freqHandler->hitCount(), freqHandler->cleanTime());
        mProcess.reset();
        handleRequest({ name(), "type"}, mBroadcast);
    });
    mFuture.get();
    send({ name(), __func__ });
}

void ExpoController::triggerCount(const Request& request, const SendCallback& send) const {
	auto expo = dynamic_cast<Exposition*>(mProcess.get());
	if(expo == nullptr)
		throw runtime_error("ExpoController::stopRead process is not expo");
	send({ name(), __func__, {expo->triggerCount()} });
}

void ExpoController::packageCount(const Request& request, const SendCallback& send) const {
	auto expo = dynamic_cast<Exposition*>(mProcess.get());
	if(expo == nullptr)
		throw runtime_error("ExpoController::stopRead process is not expo");
	send({ name(), __func__, {expo->packageCount()} });
}

void ExpoController::duration(const Request& request, const SendCallback& send) const {
    if(mProcess.get() == nullptr)
        throw runtime_error("ExpoController::stopRead process is not active");

	send({ name(), __func__, {mProcess->duration().count()} });
}

void ExpoController::freq(const Request& request, const SendCallback& send) const {
    auto freqHandler = dynamic_cast<FreqHandler*>(mProcess.get());
    if(freqHandler != nullptr) {
        assert(mFuture.valid());
        mFreq = createFreq(freqHandler->hitCount(), freqHandler->cleanTime());
    }
	send({ name(), __func__, convertFreq(mFreq) });
}

string ExpoController::getProcessType() const {
    if(dynamic_cast<Exposition*>(mProcess.get()) != nullptr)
        return "expo";
    if(dynamic_cast<FreqHandler*>(mProcess.get()) != nullptr)
        return "freq";
    return "none";
}

TrekFreq ExpoController::createFreq(TrekFreq hitCount, microseconds dur) {
    for(auto& chamCountPair : hitCount) {
		auto& chamCount = chamCountPair.second;
		for(auto& wireCount : chamCount)
            wireCount *= 1000000.0 / dur.count();
	}
    return hitCount;
}

json::array_t ExpoController::convertFreq(const TrekFreq& mFreq) {
	json::array_t jFreq;
	for(auto& chamFreqPair : mFreq) {
		auto& chamFreq = chamFreqPair.second;
		auto  chamNum  = chamFreqPair.first;
		jFreq.push_back({
			{"chamber", chamNum},
			{"freq", chamFreq},
		});
	}
	return jFreq;
}
