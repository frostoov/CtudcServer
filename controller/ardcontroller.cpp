#include "ardcontroller.hpp"

using std::make_shared;
using std::string;

using nlohmann::json;

using trek::net::Request;
using trek::net::Response;
using trek::net::Controller;

ArdController::ArdController(const std::string& name, const ModulePtr& module)
    : Controller(name, createMethods()),
      mDevice(module) { }

Controller::Methods ArdController::createMethods() {
    return {
        {"open",                  [&](const Request& query) { return this->open(query); } },
		{"close",                 [&](const Request& query) { return this->close(query); } },
		{"isOpen",                [&](const Request& query) { return this->isOpen(query); } },
		{"turnOn",                [&](const Request& query) { return this->turnOn(query); } },
		{"turnOff",               [&](const Request& query) { return this->turnOff(query); } },
		{"voltage",               [&](const Request& query) { return this->voltage(query); } },
    };
}

Response ArdController::open(const Request& request) {
    mDevice->open(request.inputs().at(0).get<string>());
    return {
        name(),
        "open",
        json::array(),
        true
    };
}

Response ArdController::close(const Request& request) {
    mDevice->close();
    return {
        name(),
        "close",
        json::array(),
        true
    };
}
Response ArdController::isOpen(const Request& request) {
    return {
        name(),
        "close",
        {mDevice->isOpen()},
        true
    };
}

Response ArdController::turnOn(const Request& request) {
    mDevice->turnOn();
    return {
        name(),
        "turnOn",
        json::array(),
        true
    };

}
Response ArdController::turnOff(const Request& request) {
    mDevice->turnOff();
    return {
        name(),
        "turnOn",
        json::array(),
        true
    };

}
Response ArdController::voltage(const Request& request) {
    return {
        name(),
        "turnOn",
        {mDevice->voltage()},
        true
    };
}
