#include "tdccontroller.hpp"

using std::make_shared;
using std::string;

using caen::Module;

using trek::data::Lsb;
using trek::data::EdgeDetection;

using nlohmann::json;

using trek::net::Request;
using trek::net::Response;
using trek::net::JController;

TdcController::TdcController(int32_t vmeAddress)
    : JController(createMethods()),
      mDevice(make_shared<Module>(vmeAddress)) { }

JController::Methods TdcController::createMethods() {
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

Response TdcController::init(const Request& request) {
    return {
        name(),
        "init",
        json::array(),
        mDevice->initialize(),
    };
}

Response TdcController::close(const Request& request) {
    return {
        name(),
        "close",
        json::array(),
        mDevice->close(),
    };
}

Response TdcController::isInit(const Request& request) {
    return {
        name(),
        "isInit",
        json::array({mDevice->isInit() }),
        true,
    };
}

Response TdcController::setLog(const Request& request) {
    mDevice->setLog(request.inputs().at(0));
    return {
        name(),
        "setLog",
        json::array(),
        true,
    };
}

Response TdcController::getLog(const Request& request) {
    json::array_t log;
    while(mDevice->hasMessages())
        log.push_back(mDevice->popMessage().first);

    return {
        name(),
        "getLog",
        log,
        true,
    };
}

Response TdcController::softwareClear(const Request& request) {
    return {
        name(),
        "softwareClear",
        json::array(),
        mDevice->softwareClear(),
    };
}

Response TdcController::getSettings(const Request& request) {
    const auto& settings = mDevice->settings();
    return {
        name(),
        "getSettings",
        {
            settings.triggerMode(),
            settings.triggerSubtraction(),
            settings.tdcMeta(),
            settings.windowWidth(),
            settings.windowOffset(),
            uint16_t(settings.edgeDetection()),
            uint16_t(settings.lsb()),
            settings.almostFull(),
            settings.control(),
            settings.status(),
            settings.deadTime(),
            settings.eventBlt()
        },
        true
    };
}

Response TdcController::updateSettings(const Request& request) {
    return {
        name(),
        "updateSettings",
        json::array(),
        mDevice->updateSettings(),
    };
}

Response TdcController::setSettings(const Request& request) {
    auto settings  = createSettings(request);
    return {
        name(),
        "setSettings",
        json::array(),
        mDevice->setSettings(settings),
    };
}

Response TdcController::setTriggerMode(const Request& request) {
    return {
        name(),
        "setTriggerMode",
        json::array(),
        mDevice->setTriggerMode(request.inputs().front()),
    };
}

Response TdcController::setTriggerSubtraction(const Request& request) {
    return {
        name(),
        "setTriggerSubtraction",
        json::array(),
        mDevice->setTriggerSubtraction(request.inputs().front()),
    };
}

Response TdcController::setTdcMeta(const Request& request) {
    return {
        name(),
        "setTdcMeta",
        json::array(),
        mDevice->setTdcMeta(request.inputs().front()),
    };
}

Response TdcController::setWindowWidth(const Request& request) {
    return {
        name(),
        "setWindowWidth",
        json::array(),
        mDevice->setWindowWidth(request.inputs().front()),
    };
}

Response TdcController::setWindowOffset(const Request& request) {
    return {
        name(),
        "setWindowOffset",
        json::array(),
        mDevice->setWindowOffset(request.inputs().front()),
    };
}

Response TdcController::setEdgeDetection(const Request& request) {
    auto edgeDetection = request.inputs().front().get<uint16_t>();
    return {
        name(),
        "setEdgeDetection",
        json::array(),
        mDevice->setEdgeDetection(static_cast<EdgeDetection>(edgeDetection)),
    };
}

Response TdcController::setLsb(const Request& request) {
    auto lsb = request.inputs().front().get<uint16_t>();
    return {
        name(),
        "setLsb",
        json::array(),
        mDevice->setLsb(static_cast<Lsb>(lsb)),
    };
}

Response TdcController::setAlmostFull(const Request& request) {
    return {
        name(),
        "setAlmostFull",
        json::array(),
        mDevice->setAlmostFull(request.inputs().front()),
    };
}

Response TdcController::setControl(const Request& request) {
    return {
        name(),
        "setControl",
        json::array(),
        mDevice->setControl(request.inputs().front()),
    };
}

Response TdcController::setDeadTime(const Request& request) {
    return {
        name(),
        "setDeadTime",
        json::array(),
        mDevice->setDeadTime(request.inputs().front()),
    };
}

Response TdcController::setEventBLT(const Request& request) {
    return {
        name(),
        "setEventBLT",
        json::array(),
        mDevice->setEventBLT(request.inputs().front()),
    };
}

trek::data::Settings TdcController::createSettings(const Request& request) const {
    trek::data::Settings settings;
    const auto& input = request.inputs();
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

const std::string& TdcController::name() const {
    static string n("tdc");
    return n;
}

TdcController::ModulePtr&TdcController::getModule() {
    return mDevice;
}
