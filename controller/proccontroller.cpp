#include "proccontroller.hpp"

#include "exposition/exposition.hpp"

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

using nlohmann::json;

using trek::Callback;
using trek::net::Request;
using trek::net::Response;
using trek::net::Controller;

ProcessController::ProcessController(const std::string& name,
                                     const ModulePtr& module,
                                     const Exposition::Settings& settings,
                                     const ChannelConfig& config)
	: Controller(name, createMethods()),
	  mDevice(module),
	  mChannelConfig(config),
	  mSettings(settings) { }

ProcessController::~ProcessController() {
	if(mFuture.valid() || mProcess != nullptr) {
		mProcess->stop();
		mFuture.get();
		mProcess.reset();
	}
}

const Callback<void(unsigned)>& ProcessController::onNewRun() {
	return mOnNewRun;
}

Controller::Methods ProcessController::createMethods() {
	return {
		{"getType",             [&](const Request & request) { return this->getType(request); } },
		{"startRead",           [&](const Request & request) { return this->startRead(request); } },
		{"stopRead",            [&](const Request & request) { return this->stopRead(request); } },
		{"triggerCount",     [&](const Request & request) { return this->triggerCount(request); } },
		{"packageCount",        [&](const Request & request) { return this->packageCount(request); } },
		{"duration",            [&](const Request & request) { return this->duration(request); } },
	};
}

Response ProcessController::getType(const Request& request) {
	return {
		name(),
		"getType",
		{getProcessType(mProcess)},
		true
	};
}

Response ProcessController::getRun(const Request& request) {
	return {
		name(),
		"getType",
		{mSettings.nRun},
		true
	};
}

Response ProcessController::startRead(const Request& request) {
	if(mProcess != nullptr || mFuture.valid())
		throw logic_error("ProcessController::startRead process already active");
	mProcess = createReadManager(request);
	mFuture = std::async(std::launch::async, [&] {mProcess->run();});
	return {
		name(),
		"startRead",
		json::array(),
		true,
	};
}

Response ProcessController::stopRead(const Request& request) {
	auto readManager = dynamic_cast<Exposition*>(mProcess.get());
	if(readManager == nullptr || !mFuture.valid())
		throw logic_error("ProcessController::stopRead process isnt reader");
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

Response ProcessController::triggerCount(const Request& request) const {
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

Response ProcessController::packageCount(const Request& request) const {
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

Response ProcessController::duration(const Request& request) const {
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


ProcessController::ProcessPtr ProcessController::createReadManager(const Request& request) const {
	return make_unique<Exposition>(
	           mDevice,
	           mSettings,
	           mChannelConfig
	       );
}

bool ProcessController::isReadManager(const ProcessPtr& processManager) const {
	if(!processManager)
		return false;
	const auto& ref = *processManager.get();
	return typeid(ref) == typeid(Exposition);
}

string ProcessController::getProcessType(const ProcessController::ProcessPtr& process) const {
	if(isReadManager(process))
		return "ctudc";
	return "null";
}
