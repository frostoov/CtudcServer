#include "tdccontroller.hpp"

using std::string;

using nlohmann::json;

using trek::net::Request;
using trek::net::Response;
using trek::net::Controller;

Caen2718Contr::Caen2718Contr(const std::string& name, const ModulePtr& module)
    : Controller(name, createMethods()),
      mDevice(module) { }

Controller::Methods Caen2718Contr::createMethods() {
    return {
        {"open",                  [&](auto& request) { return this->open(request); } },
        {"close",                 [&](auto& request) { return this->close(request); } },
        {"isOpen",                [&](auto& request) { return this->isOpen(request); } },
        {"clear",                 [&](auto& request) { return this->clear(request); } },
        {"reset",                 [&](auto& request) { return this->reset(request); } },
        {"stat",                  [&](auto& request) { return this->stat(request); } },
        {"ctrl",                  [&](auto& request) { return this->ctrl(request); } },
        {"tdcMeta",               [&](auto& request) { return this->tdcMeta(request); } },
        {"setMode",	          [&](auto& request) { return this->setMode(request); } },
        {"mode",                  [&](auto& request) { return this->mode(request); } },
        {"setWindowWidth",        [&](auto& request) { return this->setWindowWidth(request); } },
        {"setWindowOffset",       [&](auto& request) { return this->setWindowOffset(request); } },
        {"setEdgeDetection",      [&](auto& request) { return this->setEdgeDetection(request); } },
        {"setLsb",                [&](auto& request) { return this->setLsb(request);  } },
        {"setCtrl",               [&](auto& request) { return this->setCtrl(request); } },
        {"setTdcMeta",            [&](auto& request) { return this->setTdcMeta(request); } },
        {"settings",              [&](auto& request) { return this->settings(request); } },
        {"updateSettings",        [&](auto& request) { return this->updateSettings(request); } },
    };
}

Response Caen2718Contr::open(const Request&) {
    mDevice->open();
    broadcast(isOpen({}));
    return { name(), __func__ };
}

Response Caen2718Contr::close(const Request&) {
    mDevice->close();
    broadcast(isOpen({}));
    return {name(), __func__};
}

Response Caen2718Contr::isOpen(const Request&) {
    return {name(), __func__, {mDevice->isOpen()}};
}

Response Caen2718Contr::clear(const Request&) {
    mDevice->clear();
    return {name(), __func__};
}

Response Caen2718Contr::reset(const Request& request) {
    mDevice->reset();
    return {name(), __func__};
}

Response Caen2718Contr::setMode(const Request& request) {
    auto mode = request.inputs.at(0).get<int>();
    mDevice->setMode(Tdc::Mode(mode));
    broadcast(this->mode({}));
    return {name(), __func__};
}

Response Caen2718Contr::setWindowWidth(const Request& request) {
    mDevice->setWindowWidth(request.inputs.at(0));
    broadcast(settings({}));
    return {name(), __func__};
}

Response Caen2718Contr::setWindowOffset(const Request& request) {
    mDevice->setWindowOffset(request.inputs.at(0));
    broadcast(settings({}));
    return {name(), __func__};
}

Response Caen2718Contr::setEdgeDetection(const Request& request) {
    auto ed = request.inputs.at(0).get<int>();
    mDevice->setEdgeDetection(Tdc::EdgeDetection(ed));
    broadcast(settings({}));
    return {name(), __func__};
}

Response Caen2718Contr::setLsb(const Request& request) {
    mDevice->setLsb(request.inputs.at(0));
    broadcast(settings({}));
    return {name(), __func__};
}

Response Caen2718Contr::setCtrl(const Request& request) {
    mDevice->setCtrl(request.inputs.at(0));
    broadcast(ctrl({}));
    return {name(), __func__};
}

Response Caen2718Contr::setTdcMeta(const Request& request) {
    mDevice->setTdcMeta(request.inputs.at(0));
    broadcast(tdcMeta({}));
    return {name(), __func__};
}

Response Caen2718Contr::stat(const Request& request) {
    return {name(), __func__, {mDevice->stat()}};
}

Response Caen2718Contr::ctrl(const Request& request) {
    return {name(), __func__, {mDevice->ctrl()}};
}

Response Caen2718Contr::tdcMeta(const Request& request) {
    return {name(), __func__, {mDevice->tdcMeta()}};
}

Response Caen2718Contr::mode(const Request& request) {
    return {name(), __func__, {int(mDevice->mode())}};
}

Response Caen2718Contr::updateSettings(const Request& request) {
    mDevice->updateSettings();
    auto settings = mDevice->settings();
    return {
        name(), __func__,
        json::array({
            settings.windowWidth,
            settings.windowOffset,
            int(settings.edgeDetection),
            settings.lsb,
        })
    };
}

Response Caen2718Contr::settings(const Request& request) {
    auto settings = mDevice->settings();
    return {
        name(), __func__,
        json::array({
            settings.windowWidth,
            settings.windowOffset,
            int(settings.edgeDetection),
            settings.lsb,
        })
    };
}
