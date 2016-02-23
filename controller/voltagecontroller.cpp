#include "voltagecontroller.hpp"


using std::string;

using nlohmann::json;

using trek::net::Request;
using trek::net::Response;
using trek::net::Controller;


VoltageController::VoltageController(const string& name, const ModulePtr& module, const Config& config)
	: Controller(name, createMethods()),
	  mDevice(module),
	  mConfig(config) { }

Controller::Methods VoltageController::createMethods() {
	return {
		{"open",                 [&](const Request& request) { return this->open(request); } },
		{"close",                [&](const Request& request) { return this->close(request); } },
		{"isOpen",               [&](const Request& request) { return this->isOpen(request); } },
        {"turnOn2",              [&](const Request& request) { return this->turnOn2(request); } },
		{"turnOff2",             [&](const Request& request) { return this->turnOff2(request); } },
        {"setVoltage2",          [&](const Request& request) { return this->setVoltage2(request); } },
		{"voltage2",             [&](const Request& request) { return this->voltage2(request); } },

		{"turnOn12",              [&](const Request& request) { return this->turnOn12(request); } },
		{"turnOff12",             [&](const Request& request) { return this->turnOff12(request); } },
		{"voltage12",             [&](const Request& request) { return this->voltage12(request); } },
	};
}


Response VoltageController::open(const Request& request) {
	mDevice->open(request.inputs().at(0).get<string>());
	return {
		name(),
		"open",
		json::array(),
		true
	};
}

Response VoltageController::close(const Request& request) {
	mDevice->close();
	return {
		name(),
		"close",
		json::array(),
		true
	};
}

Response VoltageController::isOpen(const Request& request) {
	return {
		name(),
		"isOpen",
		{mDevice->isOpen()},
		true
	};
}


Response VoltageController::turnOn2(const Request&) {
	mDevice->turnOn(mConfig.cell2);
	return {
		name(),
		"turnOn2",
		{},
		true
	};
}

Response VoltageController::turnOff2(const Request&) {
	mDevice->turnOff(mConfig.cell2);
	return {
		name(),
		"turnOff2",
		{},
		true
	};
}

Response VoltageController::setVoltage2(const Request& request) {
	mDevice->setVoltage(mConfig.cell2, request.inputs().at(0).get<double>());
	return {
		name(),
		"setVoltage2",
		{},
		true
	};
}

Response VoltageController::voltage2(const Request& request) {
	return {
		name(),
		"voltage2",
		{mDevice->voltage(mConfig.cell2)},
		true
	};
}


Response VoltageController::turnOn12(const Request& request) {
	mDevice->turnOn(mConfig.cell12);
	mDevice->setVoltage(mConfig.cell12, 12);
	return {
		name(),
		"turnOn12",
		{},
		true
	};
}

Response VoltageController::turnOff12(const Request& request) {
	mDevice->setVoltage(mConfig.cell12, 0);
	mDevice->turnOff(mConfig.cell12);
	return {
		name(),
		"turnOff12",
		{},
		true
	};

}

Response VoltageController::voltage12(const Request& request) {
	return {
		name(),
		"voltage2",
		{mDevice->voltage(mConfig.cell12)},
		true
	};
}


json VoltageController::Config::marshal() const {
	return {
		{"cell2", cell2},
		{"cell12", cell12},
	};
}
void VoltageController::Config::unMarhsal(const json& json) {
	cell2 = json.at("cell2").get<int>();
	cell12 = json.at("cell12").get<int>();
}
