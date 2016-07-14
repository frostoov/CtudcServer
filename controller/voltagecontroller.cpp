#include "voltagecontroller.hpp"


using std::string;
using std::logic_error;

using nlohmann::json;

using trek::net::Request;
using trek::net::Response;
using trek::net::Controller;


VoltageContr::VoltageContr(const string& name, const ModulePtr& module, const FtdPtr& ftd, const Config& config)
    : Controller(name, createMethods()),
      mDevice(module),
      mFtd(ftd),
      mConfig(config) { }

Controller::Methods VoltageContr::createMethods() {
    return {
        {"open",        [&](auto& request) { return this->open(request); } },
        {"close",       [&](auto& request) { return this->close(request); } },
        {"isOpen",      [&](auto& request) { return this->isOpen(request); } },
        {"turnOn",      [&](auto& request) { return this->turnOn(request);  } },
        {"turnOff",     [&](auto& request) { return this->turnOff(request); } },
        {"stat",        [&](auto& request) { return this->stat(request); } },
        {"setVoltage",  [&](auto& request) { return this->setVoltage(request); } },
        {"setSpeedUp",  [&](auto& request) { return this->setSpeedUp(request); } },
        {"setSpeedDn",  [&](auto& request) { return this->setSpeedDn(request); } },
        {"speedUp",     [&](auto& request) { return this->speedUp(request); } },
        {"speedDn",     [&](auto& request) { return this->speedDn(request); } },
        {"voltage",     [&](auto& request) { return this->voltage(request); } },
        {"amperage",    [&](auto& request) { return this->amperage(request); } },
    };
}


Response VoltageContr::open(const Request& request) {
    mDevice->open("/dev/my_uart");
    broadcast(isOpen({}));
    return {name(), __func__};
}

Response VoltageContr::close(const Request& request) {
    mDevice->close();
    broadcast(isOpen({}));
    return {name(), __func__};
}

Response VoltageContr::isOpen(const Request& request) {
    return {name(), __func__, {mDevice->isOpen()}};
}

Response VoltageContr::stat(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    auto stat = mDevice->stat(cell);
    return {name(), __func__, {request.inputs.at(0), stat.value()}};
}

Response VoltageContr::turnOn(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    mDevice->turnOn(cell);
	broadcast(stat(request));
    return {name(), __func__};
}

Response VoltageContr::turnOff(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    mDevice->turnOff(cell);
	broadcast(stat(request));
    return {name(), __func__};
}

Response VoltageContr::setVoltage(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    auto val  = request.inputs.at(1).get<int>();
    if(cell == -1) {
        uint8_t data[] = { 0x0, 255, 0x1, uint8_t(val) };
        mFtd->write(data, sizeof(data));
    } else {
        mDevice->setVoltage(cell, val);
    }
	broadcast(voltage(request));
    return {name(), __func__};
}

Response VoltageContr::setSpeedUp(const Request& request) {
    auto cell = getCell(request.inputs.at(0));
    auto val = request.inputs.at(1).get<int>();
    mDevice->setSpeedUp(cell, val);
	broadcast(speedUp(request));
    return {name(), __func__};
}
Response VoltageContr::setSpeedDn(const Request& request) {
    auto cell = getCell(request.inputs.at(0));
    auto val = request.inputs.at(1).get<int>();
    mDevice->setSpeedDn(cell, val);
	broadcast(speedDn(request));
    return {name(), __func__};
}

Response VoltageContr::speedUp(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    return {name(), __func__, {request.inputs.at(0), mDevice->speedUp(cell)}};
}
Response VoltageContr::speedDn(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    return {name(), __func__, {request.inputs.at(0), mDevice->speedDn(cell)}};
}

Response VoltageContr::voltage(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    return {name(), __func__, {request.inputs.at(0), mDevice->voltage(cell)}};
}

Response VoltageContr::amperage(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    return {name(), __func__, {request.inputs.at(0), mDevice->amperage(cell)}};
}

int VoltageContr::getCell(const std::string& name) {
    if(name == "signal")
        return this->mConfig.signal;
    if(name == "drift")
        return this->mConfig.drift;
    throw logic_error("VoltageContr::getCell invalid cell");
}

json VoltageContr::Config::marshal() const {
    return {
        {"signal", signal},
        {"drift", drift},
    };
}
void VoltageContr::Config::unMarhsal(const json& json) {
    signal = json.at("signal").get<int>();
    drift = json.at("drift").get<int>();
}
