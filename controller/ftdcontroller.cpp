#include "ftdcontroller.hpp"

using std::string;
using std::make_shared;

using nlohmann::json;

using trek::net::JController;
using trek::net::Request;
using trek::net::Response;

FtdController::FtdController(uint32_t address)
    : JController(createMethods()),
      mDevice(make_shared<ftdi::Module>(address)) { }

const string& FtdController::name() const {
    static string n("ftd");
    return n;
}

JController::Methods FtdController::createMethods() {
    return {
        {"init",     [&](const Request& request) { return this->init(request);}},
        {"close",    [&](const Request& request) { return this->close(request);}},
        {"isInit",   [&](const Request& request) { return this->isInit(request);}},
        {"setCodes", [&](const Request& request) { return this->setCodes(request);}},
    };
}

Response FtdController::init(const Request& request) {
    if(mDevice->open("C232HM-EDHSL-0"))
        mDevice->initialize({ftdi::I2C_CLOCK_STANDARD_MODE, 1, 0});
    return {
        name(),
        "init",
        json::array(),
        mDevice->isOpen(),
    };
}

Response FtdController::close(const Request& request) {
    return {
        name(),
        "closeFtd",
        json::array(),
        mDevice->close()
    };
}

Response FtdController::isInit(const Request& request) {
    return {
        name(),
        "isInit",
        json::array({mDevice->isOpen()}),
        true
    };
}

Response FtdController::setCodes(const Request& request) {
    uint8_t data[] = {
        0x0, request.inputs().at(0).get<uint8_t>(),
        0x1, request.inputs().at(1).get<uint8_t>(),
    };
    return {
        name(),
        "setCodes",
        json::array(),
        mDevice->write(data, sizeof(data)),
    };
}

