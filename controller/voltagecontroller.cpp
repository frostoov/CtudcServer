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
        {"open",                 [&](auto & request, auto & send) { return this->open(request, send); } },
        {"close",                [&](auto & request, auto & send) { return this->close(request, send); } },
        {"isOpen",               [&](auto & request, auto & send) { return this->isOpen(request, send); } },

        {"turnOn",              [&](auto & request, auto & send) { return this->turnOn(request, send);  } },
        {"turnOff",             [&](auto & request, auto & send) { return this->turnOff(request, send); } },
        {"stat",                [&](auto & request, auto & send) { return this->stat(request, send); } },
        {"setVoltage",          [&](auto & request, auto & send) { return this->setVoltage(request, send); } },
        {"setSpeedUp",          [&](auto & request, auto & send) { return this->setSpeedUp(request, send); } },
        {"setSpeedDn",          [&](auto & request, auto & send) { return this->setSpeedDn(request, send); } },
        {"speedUp",             [&](auto & request, auto & send) { return this->speedUp(request, send); } },
        {"speedDn",             [&](auto & request, auto & send) { return this->speedDn(request, send); } },
        {"voltage",             [&](auto & request, auto & send) { return this->voltage(request, send); } },
        {"amperage",            [&](auto & request, auto & send) { return this->amperage(request, send); } },
    };
}


void VoltageController::open(const Request& request, const SendCallback& send) {
    mDevice->open("/dev/my_uart");
    send({ name(), __func__});
}

void VoltageController::close(const Request& request, const SendCallback& send) {
    mDevice->close();
    send({ name(), __func__ });
}

void VoltageController::isOpen(const Request& request, const SendCallback& send) {
    send({ name(), __func__, {mDevice->isOpen()} });
}

void VoltageController::stat(const Request& request, const SendCallback& send) {
    auto cell = getCell( request.inputs.at(0) );
    auto stat = mDevice->stat(cell);
    send({ name(), __func__, {request.inputs.at(0), stat.value()} });
}

void VoltageController::turnOn(const Request& request, const SendCallback& send) {
    auto cell = getCell( request.inputs.at(0) );
    mDevice->turnOn(cell);
    send({ name(), __func__ });
}

void VoltageController::turnOff(const Request& request, const SendCallback& send) {
    auto cell = getCell( request.inputs.at(0) );
    mDevice->turnOff(cell);
    send({ name(), __func__ });
}

void VoltageController::setVoltage(const Request& request, const SendCallback& send) {
    auto cell = getCell( request.inputs.at(0) );
    auto val  = request.inputs.at(1).get<int>();
    if(cell == -1) {
        uint8_t data[] = { 0x0, 255, 0x1, uint8_t(val) };
        mFtd->write(data, sizeof(data));
    } else {
        mDevice->setVoltage(cell, val);
    }
    send({ name(), __func__ });
}

void VoltageController::setSpeedUp(const Request& request, const SendCallback& send) {
    auto cell = getCell(request.inputs.at(0));
    auto val = request.inputs.at(1).get<int>();
    mDevice->setSpeedUp(cell, val);
    send({ name(), __func__ });
}
void VoltageController::setSpeedDn(const Request& request, const SendCallback& send) {
    auto cell = getCell(request.inputs.at(0));
    auto val = request.inputs.at(1).get<int>();
    mDevice->setSpeedDn(cell, val);
    send({ name(), __func__ });
}

void VoltageController::speedUp(const Request& request, const SendCallback& send) {
    auto cell = getCell( request.inputs.at(0) );
    send({ name(), __func__, {request.inputs.at(0), mDevice->speedUp(cell)} });
}
void VoltageController::speedDn(const Request& request, const SendCallback& send) {
    auto cell = getCell( request.inputs.at(0) );
    send({ name(), __func__, {request.inputs.at(0), mDevice->speedDn(cell)} });
}

void VoltageController::voltage(const Request& request, const SendCallback& send) {
    auto cell = getCell( request.inputs.at(0) );
    send({ name(), __func__, {request.inputs.at(0), mDevice->voltage(cell)} });
}

void VoltageController::amperage(const Request& request, const SendCallback& send) {
    auto cell = getCell( request.inputs.at(0) );
    send({ name(), __func__, {request.inputs.at(0), mDevice->amperage(cell)} });
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
