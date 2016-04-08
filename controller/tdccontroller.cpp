#include "tdccontroller.hpp"

using std::string;

using nlohmann::json;

using trek::net::Request;
using trek::net::Response;
using trek::net::Controller;

TdcController::TdcController(const std::string& name, const ModulePtr& module)
    : Controller(name, createMethods()),
      mDevice(module) { }

Controller::Methods TdcController::createMethods() {
    return {
        {"open",                  [&](auto & request, auto & send) { return this->open(request, send); } },
        {"close",                 [&](auto & request, auto & send) { return this->close(request, send); } },
        {"isOpen",                [&](auto & request, auto & send) { return this->isOpen(request, send); } },
        {"clear",                 [&](auto & request, auto & send) { return this->clear(request, send); } },
        {"reset",                 [&](auto & request, auto & send) { return this->reset(request, send); } },
        {"stat",                  [&](auto & request, auto & send) { return this->stat(request, send); } },
        {"ctrl",                  [&](auto & request, auto & send) { return this->ctrl(request, send); } },
        {"tdcMeta",               [&](auto & request, auto & send) { return this->tdcMeta(request, send); } },
        {"setMode",	              [&](auto & request, auto & send) { return this->setMode(request, send); } },
        {"mode",                  [&](auto & request, auto & send) { return this->mode(request, send); } },
        {"setWindowWidth",        [&](auto & request, auto & send) { return this->setWindowWidth(request, send); } },
        {"setWindowOffset",       [&](auto & request, auto & send) { return this->setWindowOffset(request, send); } },
        {"setEdgeDetection",      [&](auto & request, auto & send) { return this->setEdgeDetection(request, send); } },
        {"setLsb",                [&](auto & request, auto & send) { return this->setLsb(request, send);  } },
        {"setCtrl",               [&](auto & request, auto & send) { return this->setCtrl(request, send); } },
        {"setTdcMeta",            [&](auto & request, auto & send) { return this->setTdcMeta(request, send); } },
        {"settings",              [&](auto & request, auto & send) { return this->settings(request, send); } },
        {"updateSettings",        [&](auto & request, auto & send) { return this->updateSettings(request, send); } },
    };
}

void TdcController::open(const Request&, const SendCallback& send) {
    mDevice->open();
    send({ name(), __func__ });
    handleRequest({name(), "isOpen"}, mBroadcast);
}

void TdcController::close(const Request&, const SendCallback& send) {
    mDevice->close();
    broadcast({ name(), __func__ });
    handleRequest({name(), "isOpen"}, mBroadcast);
}

void TdcController::isOpen(const Request&, const SendCallback& send) {
    send({ name(), __func__, {mDevice->isOpen()} });
}

void TdcController::clear(const Request&, const SendCallback& send) {
    mDevice->clear();
    send({ name(), __func__ });
}

void TdcController::reset(const Request& request, const SendCallback& send) {
    mDevice->reset();
    send({ name(), __func__ });
}

void TdcController::setMode(const Request& request, const SendCallback& send) {
    auto mode = request.inputs.at(0).get<int>();
    mDevice->setMode(Tdc::Mode(mode));
    send({ name(), __func__ });
    handleRequest({name(), "mode"}, mBroadcast);
}

void TdcController::setWindowWidth(const Request& request, const SendCallback& send) {
    mDevice->setWindowWidth(request.inputs.at(0));
    send({ name(), __func__ });
    handleRequest({name(), "settings"}, mBroadcast);
}

void TdcController::setWindowOffset(const Request& request, const SendCallback& send) {
    mDevice->setWindowOffset(request.inputs.at(0));
    send({ name(), __func__ });
    handleRequest({name(), "settings"}, mBroadcast);
}

void TdcController::setEdgeDetection(const Request& request, const SendCallback& send) {
    auto ed = request.inputs.at(0).get<int>();
    mDevice->setEdgeDetection(Tdc::EdgeDetection(ed));
    send({ name(), __func__ });
    handleRequest({name(), "settings"}, mBroadcast);
}

void TdcController::setLsb(const Request& request, const SendCallback& send) {
    mDevice->setLsb(request.inputs.at(0));
    send({ name(), __func__ });
    handleRequest({name(), "settings"}, mBroadcast);
}

void TdcController::setCtrl(const Request& request, const SendCallback& send) {
    mDevice->setCtrl(request.inputs.at(0));
    send({ name(), __func__ });
    handleRequest({ name(), "ctrl"}, mBroadcast);
}

void TdcController::setTdcMeta(const Request& request, const SendCallback& send) {
    mDevice->setTdcMeta(request.inputs.at(0));
    send({ name(), __func__ });
    handleRequest({name(), "tdcMeta"}, mBroadcast);
}

void TdcController::stat(const Request& request, const SendCallback& send) {
    send({ name(), __func__, {mDevice->stat()} });
}

void TdcController::ctrl(const Request& request, const SendCallback& send) {
    send({ name(), __func__, {mDevice->ctrl()} });
}

void TdcController::tdcMeta(const Request& request, const SendCallback& send) {
    send({ name(), __func__, {mDevice->tdcMeta()} });
}

void TdcController::mode(const Request& request, const SendCallback& send) {
    send({ name(), __func__, {int(mDevice->mode())} });
}

void TdcController::updateSettings(const Request& request, const SendCallback& send) {
    mDevice->updateSettings();
    auto settings = mDevice->settings();
    send({
        name(), __func__,
        json::array({
            settings.windowWidth,
            settings.windowOffset,
            int(settings.edgeDetection),
            settings.lsb,
        })
    });
}

void TdcController::settings(const Request& request, const SendCallback& send) {
    auto settings = mDevice->settings();
    send({
        name(), __func__,
        json::array({
            settings.windowWidth,
            settings.windowOffset,
            int(settings.edgeDetection),
            settings.lsb,
        })
    });
}
