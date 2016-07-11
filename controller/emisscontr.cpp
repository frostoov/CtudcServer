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
        {"open",    [&](auto & request) { return this->open(request); } },
        {"close",   [&](auto & request) { return this->close(request); } },
        {"isOpen",  [&](auto & request) { return this->isOpen(request); } },
        {"clear",   [&](auto & request) { return this->clear(request); } },
    };
}

Response EmissContr::open(const Request&) {
    mDevice->open();
    broadcast(isOpen({}));
    return {name(), __func__};
}

Response EmissContr::close(const Request&) {
    mDevice->close();
    broadcast(isOpen({}));
    return {name(), __func__};
}

Response EmissContr::isOpen(const Request&) {
    return {name(), __func__, {mDevice->isOpen()}};
}

Response EmissContr::clear(const Request&) {
    mDevice->clear();
    return { name(), __func__ };
}

Response EmissContr::settings(const Request& request) {
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
