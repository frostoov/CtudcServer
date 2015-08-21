#include "tdccontroller.hpp"

namespace ctudc {

using std::make_shared;

using caen::Module;
using trekdata::Lsb;
using trekdata::EdgeDetection;

using nlohmann::json;

TdcController::TdcController(int32_t vmeAddress)
    : CtudcController(createMethods()),
      mDevice(make_shared<Module>(vmeAddress)),
      mName("tdc") { }

CtudcController::Methods TdcController::createMethods() {
    return {
        {"init",               [&](const Request& query) { return this->init(query);}},
        {"close",              [&](const Request& query) { return this->close(query);}},
        {"isInit",             [&](const Request& query) { return this->isInit(query);}},

        {"softwareClear",         [&](const Request& query) { return this->softwareClear(query);}},
        {"getSettings",           [&](const Request& query) { return this->getSettings(query);}},
        {"setSettings",           [&](const Request& query) { return this->setSettings(query);}},
        {"updateSettings",        [&](const Request& query) { return this->updateSettings(query);}},
        {"getLog",                [&](const Request& query) { return this->getLog(query);}},
        {"setLog",                [&](const Request& query) { return this->setLog(query);}},
        {"setTriggerMode",	      [&](const Request& query) { return this->setTriggerMode(query);}},
        {"setTriggerSubtraction", [&](const Request& query) { return this->setTriggerSubtraction(query);}},
        {"setTdcMeta",            [&](const Request& query) { return this->setTdcMeta(query);}},
        {"setWindowWidth",        [&](const Request& query) { return this->setWindowWidth(query);}},
        {"setWindowOffset",       [&](const Request& query) { return this->setWindowOffset(query);}},
        {"setEdgeDetection",      [&](const Request& query) { return this->setEdgeDetection(query);}},
        {"setLsb",                [&](const Request& query) { return this->setLsb(query);}},
        {"setAlmostFull",         [&](const Request& query) { return this->setAlmostFull(query);}},
        {"setControl",            [&](const Request& query) { return this->setControl(query);}},
        {"setDeadTime",           [&](const Request& query) { return this->setDeadTime(query);}},
        {"setEventBLT",           [&](const Request& query) { return this->setEventBLT(query);}},
    };
}

CtudcController::Response TdcController::init(const Request& request) {
    return {
        getName(),
        "init",
        json::array(),
        mDevice->initialize(),
    };
}

CtudcController::Response TdcController::close(const Request& request) {
    return {
        getName(),
        "close",
        json::array(),
        mDevice->close(),
    };
}

CtudcController::Response TdcController::isInit(const Request& request) {
    return {
        getName(),
        "isInit",
        json::array({mDevice->isInit() }),
        true,
    };
}

CtudcController::Response TdcController::setLog(const Request& request) {
    mDevice->setLog(request.getInputs().at(0));
    return {
        getName(),
        "setLog",
        json::array(),
        true,
    };
}

CtudcController::Response TdcController::getLog(const Request& request) {
    json::array_t log;
    while(mDevice->hasMessages())
        log.push_back(mDevice->popMessage().first);

    return {
        getName(),
        "getLog",
        log,
        true,
    };
}

CtudcController::Response TdcController::softwareClear(const Request& request) {
    return {
        getName(),
        "softwareClear",
        json::array(),
        mDevice->softwareClear(),
    };
}

CtudcController::Response TdcController::getSettings(const Request& request) {
    const auto& settings = mDevice->getSettings();
    return {
        getName(),
        "getSettings",
        {
            settings.getTriggerMode(),
            settings.getTriggerSubtraction(),
            settings.getTdcMeta(),
            settings.getWindowWidth(),
            settings.getWindowOffset(),
            uint16_t(settings.getEdgeDetection()),
            uint16_t(settings.getLsb()),
            settings.getAlmostFull(),
            settings.getControl(),
            settings.getStatus(),
            settings.getDeadTime(),
            settings.getEventBLT()
        },
        true
    };
}

CtudcController::Response TdcController::updateSettings(const Request& request) {
    return {
        getName(),
        "updateSettings",
        json::array(),
        mDevice->updateSettings(),
    };
}

CtudcController::Response TdcController::setSettings(const Request& request) {
    auto settings  = createSettings(request);
    return {
        getName(),
        "setSettings",
        json::array(),
        mDevice->setSettings(settings),
    };
}

CtudcController::Response TdcController::setTriggerMode(const Request& request) {
    return {
        getName(),
        "setTriggerMode",
        json::array(),
        mDevice->setTriggerMode(request.getInputs().front()),
    };
}

CtudcController::Response TdcController::setTriggerSubtraction(const Request& request) {
    return {
        getName(),
        "setTriggerSubtraction",
        json::array(),
        mDevice->setTriggerSubtraction(request.getInputs().front()),
    };
}

CtudcController::Response TdcController::setTdcMeta(const Request& request) {
    return {
        getName(),
        "setTdcMeta",
        json::array(),
        mDevice->setTdcMeta(request.getInputs().front()),
    };
}

CtudcController::Response TdcController::setWindowWidth(const Request& request) {
    return {
        getName(),
        "setWindowWidth",
        json::array(),
        mDevice->setWindowWidth(request.getInputs().front()),
    };
}

CtudcController::Response TdcController::setWindowOffset(const Request& request) {
    return {
        getName(),
        "setWindowOffset",
        json::array(),
        mDevice->setWindowOffset(request.getInputs().front()),
    };
}

CtudcController::Response TdcController::setEdgeDetection(const Request& request) {
    auto edgeDetection = request.getInputs().front().get<uint16_t>();
    return {
        getName(),
        "setEdgeDetection",
        json::array(),
        mDevice->setEdgeDetection(static_cast<EdgeDetection>(edgeDetection)),
    };
}

CtudcController::Response TdcController::setLsb(const Request& request) {
    auto lsb = request.getInputs().front().get<uint16_t>();
    return {
        getName(),
        "setLsb",
        json::array(),
        mDevice->setLsb(static_cast<Lsb>(lsb)),
    };
}

CtudcController::Response TdcController::setAlmostFull(const Request& request) {
    return {
        getName(),
        "setAlmostFull",
        json::array(),
        mDevice->setAlmostFull(request.getInputs().front()),
    };
}

CtudcController::Response TdcController::setControl(const Request& request) {
    return {
        getName(),
        "setControl",
        json::array(),
        mDevice->setControl(request.getInputs().front()),
    };
}

CtudcController::Response TdcController::setDeadTime(const Request& request) {
    return {
        getName(),
        "setDeadTime",
        json::array(),
        mDevice->setDeadTime(request.getInputs().front()),
    };
}

CtudcController::Response TdcController::setEventBLT(const Request& request) {
    return {
        getName(),
        "setEventBLT",
        json::array(),
        mDevice->setEventBLT(request.getInputs().front()),
    };
}

trekdata::Settings TdcController::createSettings(const Request& request) const {
    trekdata::Settings settings;
    const auto& input = request.getInputs();
    settings.setTriggerMode(input.at(0));
    settings.setTriggerSubtraction(input.at(1));
    settings.setTdcMeta(input.at(2));
    settings.setWindowWidth(input.at(3));
    settings.setWindowOffset(input.at(4));
    uint16_t lsb = input.at(5);
    settings.setEdgeDetection(EdgeDetection(lsb));
    uint16_t edgeDetection = input.at(5);
    settings.setLsb(Lsb(edgeDetection));
    settings.setAlmostFull(input.at(7));
    settings.setControlRegister(input.at(8));
    settings.setDeadTime(input.at(9));
    settings.setEventBLT(input.at(10));
    return settings;
}

const std::string& TdcController::getName() const {
    return mName;
}

TdcController::ModulePtr& TdcController::getModule() {
    return mDevice;
}

}
