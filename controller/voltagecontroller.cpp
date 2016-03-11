#include "voltagecontroller.hpp"


using std::string;
using std::logic_error;

using nlohmann::json;

using trek::net::Request;
using trek::net::Response;
using trek::net::Controller;


VoltageController::VoltageController(const string& name, const ModulePtr& module, const FtdPtr& ftd, const Config& config)
	: Controller(name, createMethods()),
	  mDevice(module),
	  mFtd(ftd),
	  mConfig(config) { }

Controller::Methods VoltageController::createMethods() {
	return {
		{"open",                 [&](const Request& request) { return this->open(request); } },
		{"close",                [&](const Request& request) { return this->close(request); } },
		{"isOpen",               [&](const Request& request) { return this->isOpen(request); } },

        {"turnOn",              [&](const Request& request) { return this->turnOn(request);  } },
		{"turnOff",             [&](const Request& request) { return this->turnOff(request); } },
		{"stat",                [&](const Request& request) { return this->stat(request); } },
        {"setVoltage",          [&](const Request& request) { return this->setVoltage(request); } },
		{"setSpeedUp",          [&](const Request& request) { return this->setSpeedUp(request); } },
		{"setSpeedDn",          [&](const Request& request) { return this->setSpeedDn(request); } },
		{"speedUp",             [&](const Request& request) { return this->speedUp(request); } },
		{"speedDn",             [&](const Request& request) { return this->speedDn(request); } },
		{"voltage",             [&](const Request& request) { return this->voltage(request); } },
		{"amperage",            [&](const Request& request) { return this->amperage(request); } },
	};
}


Response VoltageController::open(const Request& request) {
	mDevice->open("/dev/my_uart");
	return { name(), __func__};
}

Response VoltageController::close(const Request& request) {
	mDevice->close();
	return { name(), __func__ };
}

Response VoltageController::isOpen(const Request& request) {
	return { name(), __func__, {mDevice->isOpen()} };
}


Response VoltageController::stat(const Request& request) {
	auto cell = getCell( request.inputs.at(0) );
	auto stat = mDevice->stat(cell);
	return { name(), __func__, {stat.value()} };
}

Response VoltageController::turnOn(const Request& request) {
	auto cell = getCell( request.inputs.at(0) );
	mDevice->turnOn(cell);
	return { name(), __func__ };
}

Response VoltageController::turnOff(const Request& request) {
	auto cell = getCell( request.inputs.at(0) );
	mDevice->turnOff(cell);
	return { name(), __func__ };
}

Response VoltageController::setVoltage(const Request& request) {
	auto cell = getCell( request.inputs.at(0) );
	auto val  = request.inputs.at(1).get<int>();
	if(cell == -1) {
		uint8_t data[] = { 0x0, 255, 0x1, uint8_t(val) };
		mFtd->write(data, sizeof(data));
	} else {
		mDevice->setVoltage(cell, val);
	}
	return { name(), __func__ };
}

Response VoltageController::setSpeedUp(const Request& request) {
	auto cell = getCell(request.inputs.at(0));
	auto val = request.inputs.at(1).get<int>();
	mDevice->setSpeedUp(cell, val);
	return { name(), __func__ };
}
Response VoltageController::setSpeedDn(const Request& request) {
	auto cell = getCell(request.inputs.at(0));
	auto val = request.inputs.at(1).get<int>();
	mDevice->setSpeedDn(cell, val);
	return { name(), __func__ };
}

Response VoltageController::speedUp(const Request& request) {
	auto cell = getCell( request.inputs.at(0) );
	return { name(), __func__, {mDevice->speedUp(cell)} };
}
Response VoltageController::speedDn(const Request& request) {
	auto cell = getCell( request.inputs.at(0) );
	return { name(), __func__, {mDevice->speedDn(cell)} };
}

Response VoltageController::voltage(const Request& request) {
	auto cell = getCell( request.inputs.at(0) );
	return { name(), __func__, {mDevice->voltage(cell)} };
}

Response VoltageController::amperage(const Request& request) {
	auto cell = getCell( request.inputs.at(0) );
	return { name(), __func__, {mDevice->amperage(cell)} };
}

int VoltageController::getCell(const std::string& name) {
	if(name == "signal")
		return this->mConfig.signal;
	if(name == "drift")
		return this->mConfig.drift;
	throw logic_error("VoltageController::getCell invalid cell");
}

json VoltageController::Config::marshal() const {
	return {
		{"signal", signal},
		{"drift", drift},
	};
}
void VoltageController::Config::unMarhsal(const json& json) {
	signal = json.at("signal").get<int>();
	drift = json.at("drift").get<int>();
}
