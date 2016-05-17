#include "emisscontr.hpp"

using std::string;

using nlohmann::json;

using trek::net::Request;
using trek::net::Response;
using trek::net::Controller;

EmissContr::EmissContr(const std::string& name, const ModulePtr& module)
    : Controller(name, createMethods()),
      mDevice(module) { }

Controller::Methods EmissContr::createMethods() {
    return {
        {"open",                  [&](auto & request, auto & send) { return this->open(request, send); } },
        {"close",                 [&](auto & request, auto & send) { return this->close(request, send); } },
        {"isOpen",                [&](auto & request, auto & send) { return this->isOpen(request, send); } },
        {"stat",                  [&](auto& request, auto& send)   { return this->stat(request, send); } },
        {"clear",                 [&](auto & request, auto & send) { return this->clear(request, send); } },
    };
}

void EmissContr::open(const Request&, const SendCallback& send) {
    mDevice->open();
    send({ name(), __func__ });
    handleRequest({name(), "isOpen"}, mBroadcast);
}

void EmissContr::close(const Request&, const SendCallback& send) {
    mDevice->close();
    broadcast({ name(), __func__ });
    handleRequest({name(), "isOpen"}, mBroadcast);
}

void EmissContr::isOpen(const Request&, const SendCallback& send) {
    send({ name(), __func__, {mDevice->isOpen()} });
}

void EmissContr::clear(const Request&, const SendCallback& send) {
    mDevice->clear();
    send({ name(), __func__ });
}

void EmissContr::stat(const Request&, const SendCallback& send) {
    send({name(), __func__, {mDevice->stat()}});
}

void EmissContr::settings(const Request& request, const SendCallback& send) {
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
