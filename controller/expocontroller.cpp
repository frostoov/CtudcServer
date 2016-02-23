#include "expocontroller.hpp"

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
	  mSettings(settings) { }

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
		{"getType",      [&](const Request & request) { return this->getType(request); } },
		{"startRead",    [&](const Request & request) { return this->startRead(request); } },
		{"stopRead",     [&](const Request & request) { return this->stopRead(request); } },
		{"startFreq",    [&](const Request & request) { return this->startFreq(request); } },
		{"stopFreq",     [&](const Request & request) { return this->stopFreq(request); } },
		{"triggerCount", [&](const Request & request) { return this->triggerCount(request); } },
		{"packageCount", [&](const Request & request) { return this->packageCount(request); } },
		{"duration",     [&](const Request & request) { return this->duration(request); } },
	};
}

Response ExpoController::getType(const Request& request) {
	return {
		name(),
		"getType",
		{getProcessType(mProcess)},
		true
	};
}

Response ExpoController::getRun(const Request& request) {
	return {
		name(),
		"getType",
		{mSettings.nRun},
		true
	};
}

Response ExpoController::startRead(const Request& request) {
	if(mProcess != nullptr || mFuture.valid())
		throw logic_error("ProcessController::startRead process already active");
	mProcess = make_unique<Exposition>(mDevice, mSettings, mChannelConfig);
	mFuture = std::async(std::launch::async, [&] {mProcess->run();});
	return {
		name(),
		"startRead",
		json::array(),
		true,
	};
}

Response ExpoController::stopRead(const Request& request) {
	auto readManager = dynamic_cast<Exposition*>(mProcess.get());
	if(readManager == nullptr || !mFuture.valid())
		throw logic_error("ProcessController::stopRead process isn not Exposition");
	readManager->stop();
	mFuture.get();
	mProcess.reset();
	++mSettings.nRun;
	mOnNewRun(mSettings.nRun);
	return {
		name(),
		"stopRead",
		json::array(),
		true
	};
}

Response ExpoController::startFreq(const Request& request) {
	if(mProcess != nullptr || mFuture.valid())
		throw logic_error("ProcessController::startFreq process already active");
	auto delay = request.inputs().at(0).get<int>();
	if(delay <= 0)
		throw logic_error("ProcessController::startFreq invlid delay value");
	mProcess = make_unique<FreqHandler>(mDevice, mChannelConfig, microseconds(delay));
	mFuture = std::async(std::launch::async, [&] {mProcess->run();});
	return {
		name(),
		"startFreq",
		json::array(),
		true,
	};
}

Response ExpoController::stopFreq(const Request& request) {
	auto freqHandler = dynamic_cast<FreqHandler*>(mProcess.get());
	if(freqHandler == nullptr || !mFuture.valid())
		throw logic_error("ProcessController::stopRead process isn not FreqHandler");
	freqHandler->stop();
	mFuture.get();
	mProcess.reset();
	return {
		name(),
		"stopRead",
		createFreqs( freqHandler->freq() ),
		true
	};
}

Response ExpoController::triggerCount(const Request& request) const {
	auto expo = dynamic_cast<Exposition*>(mProcess.get());
	if(expo == nullptr)
		throw runtime_error("ProcessController::stopRead process isnt reader");
	return {
		name(),
		"triggerCount",
		{expo->triggerCount()},
		true
	};
}

Response ExpoController::packageCount(const Request& request) const {
	auto expo = dynamic_cast<Exposition*>(mProcess.get());
	if(expo == nullptr)
		throw runtime_error("ProcessController::stopRead process isnt reader");
	return {
		name(),
		"packageCount",
		{expo->packageCount()},
		true
	};
}

Response ExpoController::duration(const Request& request) const {
	auto expo = dynamic_cast<Exposition*>(mProcess.get());
	if(expo == nullptr)
		throw runtime_error("ProcessController::duration process isnt active");
	return {
		name(),
		"duration",
		{expo->duration()},
		true,
	};
}

bool ExpoController::isReadManager(const ProcessPtr& processManager) const {
	if(!processManager)
		return false;
	const auto& ref = *processManager.get();
	return typeid(ref) == typeid(Exposition);
}

string ExpoController::getProcessType(const ExpoController::ProcessPtr& process) const {
	if(isReadManager(process))
		return "ctudc";
	return "null";
}

json::array_t ExpoController::createFreqs(const FreqHandler::TrekFreq& freq) {
	json::array_t jFreq;
	for(const auto& chamFreqPair : freq) {
		auto& chamFreq = chamFreqPair.second;
		auto  chamNum  = chamFreqPair.first;
		jFreq.push_back({
			{"chamber", chamNum},
			{"freq", chamFreq},
		});
	}
	return jFreq;
}
