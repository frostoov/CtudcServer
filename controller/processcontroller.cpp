#include "processcontroller.hpp"

#include "managers/readmanager.hpp"

#include <trek/net/request.hpp>
#include <trek/net/response.hpp>
#include <trek/net/controller.hpp>

#include <json.hpp>

using std::make_unique;
using std::chrono::microseconds;
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
                                     const ReadManager::Settings& settings,
                                     const ChannelConfig& config)
	: Controller(name, createMethods()),
	  mDevice(module),
	  mChannelConfig(config),
	  mSettings(settings) { }

const Callback<void(unsigned)>& ProcessController::onNewRun() {
	return mOnNewRun;
}

Controller::Methods ProcessController::createMethods() {
	return {
		{"getType",             [&](const Request & request) { return this->getType(request); } },
		{"startRead",           [&](const Request & request) { return this->startRead(request); } },
		{"stopRead",            [&](const Request & request) { return this->stopRead(request); } },
		{"getTriggerFrequency", [&](const Request & request) { return this->getTriggerFrequency(request); } },
		{"getPackageFrequency", [&](const Request & request) { return this->getPackageFrequency(request); } },
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
	if(mProcess)
		throw logic_error("ProcessController::startRead process already active");
	mProcess = createReadManager(request);
	mProcess->run();
	return {
		name(),
		"startRead",
		json::array(),
		true,
	};
}

Response ProcessController::stopRead(const Request& request) {
	if(!isReadManager(mProcess))
		throw logic_error("ProcessController::stopRead process isnt reader");
	mProcess->stop();
	++mSettings.nRun;
	mOnNewRun(mSettings.nRun);
	return {
		name(),
		"stopRead",
		json::array(),
		true
	};
}

Response ProcessController::getTriggerFrequency(const Request& request) const {
	if(!isReadManager(mProcess))
		throw logic_error("ProcessController::stopRead process isnt reader");
	auto ctudcManager = dynamic_cast<ReadManager*>(mProcess.get());
	auto freq = ctudcManager->getTriggerFrequency();
	return {
		name(),
		"getTriggerFrequency",
		{freq},
		true
	};
}

Response ProcessController::getPackageFrequency(const Request& request) const {
	if(!isReadManager(mProcess))
		throw logic_error("ProcessController::stopRead process isnt reader");
	auto ctudcManager = dynamic_cast<ReadManager*>(mProcess.get());
	auto freq = ctudcManager->getPackageFrequency();
	return {
		name(),
		"getPackageFrequency",
		json::array({freq}),
		true
	};
}

ProcessController::ProcessPtr ProcessController::createReadManager(const Request& request) const {
	return make_unique<ReadManager>(
		mDevice,
        mSettings,
		mChannelConfig
	);
}

bool ProcessController::isReadManager(const ProcessPtr& processManager) const {
	if(!processManager)
		return false;
	const auto& ref = *processManager.get();
	return typeid(ref) == typeid(ReadManager);
}

string ProcessController::getProcessType(const ProcessController::ProcessPtr &process) const {
	if(isReadManager(process))
		return "ctudc";
	return "null";
}
