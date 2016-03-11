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
	if(mFuture.valid() || mProcess != nullptr) {
		mProcess->stop();
		mFuture.get();
		mProcess.reset();
	}
}

const Callback<void(unsigned)>& ExpoController::onNewRun() {
	return mOnNewRun;
}

Controller::Methods ExpoController::createMethods() {
	return {
		{"type",         [&](const Request & request) { return this->type(request); } },
        {"run",          [&](const Request & request) { return this->run(request); } },
		{"startRead",    [&](const Request & request) { return this->startRead(request); } },
		{"stopRead",     [&](const Request & request) { return this->stopRead(request); } },
		{"startFreq",    [&](const Request & request) { return this->startFreq(request); } },
		{"stopFreq",     [&](const Request & request) { return this->stopFreq(request); } },
		{"triggerCount", [&](const Request & request) { return this->triggerCount(request); } },
		{"packageCount", [&](const Request & request) { return this->packageCount(request); } },
		{"duration",     [&](const Request & request) { return this->duration(request); } },
        {"freq",         [&](const Request & request) { return this->freq(request); } },
	};
}

Response ExpoController::type(const Request& request) {
	return { name(), __func__, {getProcessType()} };
}

Response ExpoController::run(const Request& request) {
	return { name(), __func__, {mConfig.nRun} };
}

Response ExpoController::startRead(const Request& request) {
	if(mFuture.valid())
		throw logic_error("ExpoController::startRead process is active");
	mProcess = make_unique<Exposition>(mDevice, mConfig, mChannelConfig);
	mFuture = std::async(std::launch::async, [&] {mProcess->run();});
	return { name(), __func__ };
}

Response ExpoController::stopRead(const Request& request) {
    if(!mFuture.valid())
        throw logic_error("ExpoController::stopRead process is not active");
	auto readManager = dynamic_cast<Exposition*>(mProcess.get());
	if(readManager == nullptr)
        throw logic_error("ExpoController::stopRead process already active");
    readManager->stop();
    auto f = gsl::finally([&]{
        mProcess.reset();
    });
    mFuture.get();
	++mConfig.nRun;
	mOnNewRun(mConfig.nRun);
	return { name(), __func__ };
}

Response ExpoController::startFreq(const Request& request) {
	if(mFuture.valid())
		throw logic_error("ExpoController::stopRead process is active");
	auto delay = request.inputs.at(0).get<int>();
	if(delay <= 0)
		throw logic_error("ExpoController::startFreq invalid delay value");
	mProcess = make_unique<FreqHandler>(mDevice, mChannelConfig, microseconds(delay));
	mFuture = std::async(std::launch::async, [&] {mProcess->run();});
	return { name(), __func__ };
}

Response ExpoController::stopFreq(const Request& request) {
    if(!mFuture.valid())
        throw logic_error("ExpoController::stopFreq process is not active");
	auto freqHandler = dynamic_cast<FreqHandler*>(mProcess.get());
	if(freqHandler == nullptr || !mFuture.valid())
		throw logic_error("ExpoController::stopFreq process is not freq");
	freqHandler->stop();
    auto f = gsl::finally([&]{
        mFreq = createFreq(freqHandler->hitCount(), freqHandler->cleanTime());
        mProcess.reset();
    });
    mFuture.get();
	return { name(), __func__ };
}

Response ExpoController::triggerCount(const Request& request) const {
	auto expo = dynamic_cast<Exposition*>(mProcess.get());
	if(expo == nullptr)
		throw runtime_error("ExpoController::stopRead process is not expo");
	return { name(), __func__, {expo->triggerCount()} };
}

Response ExpoController::packageCount(const Request& request) const {
	auto expo = dynamic_cast<Exposition*>(mProcess.get());
	if(expo == nullptr)
		throw runtime_error("ExpoController::stopRead process is not expo");
	return { name(), __func__, {expo->packageCount()} };
}

Response ExpoController::duration(const Request& request) const {
    if(mProcess.get() == nullptr)
        throw runtime_error("ExpoController::stopRead process is not active");

	return { name(), __func__, {mProcess->duration().count()} };
}

Response ExpoController::freq(const Request& request) const {
    auto freqHandler = dynamic_cast<FreqHandler*>(mProcess.get());
    if(freqHandler != nullptr) {
        assert(mFuture.valid());
        mFreq = createFreq(freqHandler->hitCount(), freqHandler->cleanTime());
    }
	return { name(), __func__, convertFreq(mFreq) };
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
