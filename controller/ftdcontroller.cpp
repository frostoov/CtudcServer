#include "ftdcontroller.hpp"

namespace ctudc {

using std::make_shared;

using nlohmann::json;

FtdController::FtdController(uint32_t address)
    : CtudcController(createMethods()),
      mDevice(make_shared<ftdi::Module>(address)),
      mName("ftd") { }

const std::string& FtdController::getName() const {
    return mName;
}

CtudcController::Methods FtdController::createMethods() {
    return {
        {"init",     [&](const Request& request) { return this->init(request);}},
        {"close",    [&](const Request& request) { return this->close(request);}},
        {"isInit",   [&](const Request& request) { return this->isInit(request);}},
        {"setCodes", [&](const Request& request) { return this->setCodes(request);}},
    };
}

CtudcController::Response FtdController::init(const Request& request) {
    if(mDevice->open("C232HM-EDHSL-0"))
        mDevice->initialize({ftdi::I2C_CLOCK_STANDARD_MODE, 1, 0});
    return {
        getName(),
        "init",
        json::array(),
        mDevice->isOpen(),
    };
}

CtudcController::Response FtdController::close(const Request& request) {
    return {
        getName(),
        "closeFtd",
        json::array(),
        mDevice->close()
    };
}

CtudcController::Response FtdController::isInit(const Request& request) {
    return {
        getName(),
        "isInit",
        json::array({mDevice->isOpen()}),
        true
    };
}

CtudcController::Response FtdController::setCodes(const Request& request) {
    uint8_t data[] = {
        0x0, request.getInputs().at(0).get<uint8_t>(),
        0x1, request.getInputs().at(1).get<uint8_t>(),
    };
    return {
        getName(),
        "setCodes",
        json::array(),
        mDevice->write(data, sizeof(data)),
    };
}

}
