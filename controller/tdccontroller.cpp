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

void Caen2718Contr::open(const Request&, const SendCallback& send) {
    mDevice->open();
    send({ name(), __func__ });
    handleRequest({name(), "isOpen"}, mBroadcast);
}

void Caen2718Contr::close(const Request&, const SendCallback& send) {
    mDevice->close();
    broadcast({ name(), __func__ });
    handleRequest({name(), "isOpen"}, mBroadcast);
}

void Caen2718Contr::isOpen(const Request&, const SendCallback& send) {
    send({ name(), __func__, {mDevice->isOpen()} });
}

void Caen2718Contr::clear(const Request&, const SendCallback& send) {
    mDevice->clear();
    send({ name(), __func__ });
}

void Caen2718Contr::reset(const Request& request, const SendCallback& send) {
    mDevice->reset();
    send({ name(), __func__ });
}

void Caen2718Contr::setMode(const Request& request, const SendCallback& send) {
    auto mode = request.inputs.at(0).get<int>();
    mDevice->setMode(Tdc::Mode(mode));
    send({ name(), __func__ });
    handleRequest({name(), "mode"}, mBroadcast);
}

void Caen2718Contr::setWindowWidth(const Request& request, const SendCallback& send) {
    mDevice->setWindowWidth(request.inputs.at(0));
    send({ name(), __func__ });
    handleRequest({name(), "settings"}, mBroadcast);
}

void Caen2718Contr::setWindowOffset(const Request& request, const SendCallback& send) {
    mDevice->setWindowOffset(request.inputs.at(0));
    send({ name(), __func__ });
    handleRequest({name(), "settings"}, mBroadcast);
}

void Caen2718Contr::setEdgeDetection(const Request& request, const SendCallback& send) {
    auto ed = request.inputs.at(0).get<int>();
    mDevice->setEdgeDetection(Tdc::EdgeDetection(ed));
    send({ name(), __func__ });
    handleRequest({name(), "settings"}, mBroadcast);
}

void Caen2718Contr::setLsb(const Request& request, const SendCallback& send) {
    mDevice->setLsb(request.inputs.at(0));
    send({ name(), __func__ });
    handleRequest({name(), "settings"}, mBroadcast);
}

void Caen2718Contr::setCtrl(const Request& request, const SendCallback& send) {
    mDevice->setCtrl(request.inputs.at(0));
    send({ name(), __func__ });
    handleRequest({ name(), "ctrl"}, mBroadcast);
}

void Caen2718Contr::setTdcMeta(const Request& request, const SendCallback& send) {
    mDevice->setTdcMeta(request.inputs.at(0));
    send({ name(), __func__ });
    handleRequest({name(), "tdcMeta"}, mBroadcast);
}

void Caen2718Contr::stat(const Request& request, const SendCallback& send) {
    send({ name(), __func__, {mDevice->stat()} });
}

void Caen2718Contr::ctrl(const Request& request, const SendCallback& send) {
    send({ name(), __func__, {mDevice->ctrl()} });
}

void Caen2718Contr::tdcMeta(const Request& request, const SendCallback& send) {
    send({ name(), __func__, {mDevice->tdcMeta()} });
}

void Caen2718Contr::mode(const Request& request, const SendCallback& send) {
    send({ name(), __func__, {int(mDevice->mode())} });
}

void Caen2718Contr::updateSettings(const Request& request, const SendCallback& send) {
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

void Caen2718Contr::settings(const Request& request, const SendCallback& send) {
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
