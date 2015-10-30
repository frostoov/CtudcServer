#include "tdccontroller.hpp"

#include <iostream>

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
		{"open",                  [&](const Request& query) { return this->open(query); } },
		{"close",                 [&](const Request& query) { return this->close(query); } },
		{"isOpen",                [&](const Request& query) { return this->isOpen(query); } },
		{"clear",                 [&](const Request& query) { return this->clear(query); } },
		{"reset",                 [&](const Request& query) { return this->reset(query); } },
		{"stat",                  [&](const Request& query) { return this->stat(query); } },
		{"ctrl",                  [&](const Request& query) { return this->ctrl(query); } },
		{"setMode",	              [&](const Request& query) { return this->setMode(query); } },
		{"setWindowWidth",        [&](const Request& query) { return this->setWindowWidth(query); } },
		{"setWindowOffset",       [&](const Request& query) { return this->setWindowOffset(query); } },
		{"setEdgeDetection",      [&](const Request& query) { return this->setEdgeDetection(query); } },
		{"setLsb",                [&](const Request& query) { return this->setLsb(query);  } },
		{"setCtrl",               [&](const Request& query) { return this->setCtrl(query); } },
		{"setTdcMeta",            [&](const Request& query) { return this->setTdcMeta(query); } },
	};
}

Response TdcController::open(const Request& request) {
	mDevice->open();
	return {
		name(),
		"init",
		json::array(),
		true,
	};
}

Response TdcController::close(const Request& request) {
	mDevice->close();
	return {
		name(),
		"close",
		json::array(),
		true,
	};
}

Response TdcController::isOpen(const Request& request) {
	return {
		name(),
		"isInit",
		{mDevice->isOpen()},
		true,
	};
}

Response TdcController::clear(const Request &request) {
	mDevice->clear();
	return {
		name(),
		"clear",
		json::array(),
		true
	};
}

Response TdcController::reset(const Request &request) {
	mDevice->reset();
	return {
		name(),
		"reset",
		json::array(),
		true,
	};
}

Response TdcController::setMode(const Request& request) {
	auto mode = request.inputs().at(0).get<int>();
	mDevice->setMode(Tdc::Mode(mode));
	return {
		name(),
		"setTriggerMode",
		json::array(),
		true
	};
}

Response TdcController::setWindowWidth(const Request& request) {
	mDevice->setWindowWidth(request.inputs().at(0));
	return {
		name(),
		"setWindowWidth",
		json::array(),
		true,
	};
}

Response TdcController::setWindowOffset(const Request& request) {
	mDevice->setWindowOffset(request.inputs().at(0));
	return {
		name(),
		"setWindowOffset",
		json::array(),
		true,
	};
}

Response TdcController::setEdgeDetection(const Request& request) {
	auto ed = request.inputs().at(0).get<int>();
	mDevice->setEdgeDetection(Tdc::EdgeDetection(ed));
	return {
		name(),
		"setEdgeDetection",
		json::array(),
		true,
	};
}

Response TdcController::setLsb(const Request& request) {
	mDevice->setLsb(request.inputs().at(0));
	return {
		name(),
		"setLsb",
		json::array(),
		true,
	};
}

Response TdcController::setCtrl(const Request& request) {
	mDevice->setCtrl(request.inputs().at(0));
	return {
		name(),
		"setControl",
		json::array(),
		true,
	};
}

Response TdcController::stat(const Request& request) {
	return {
		name(),
		"stat",
		{mDevice->stat()},
		true
	};
}

Response TdcController::ctrl(const Request& request) {
	return {
		name(),
		"ctrl",
		{mDevice->ctrl()},
		true
	};
}

Response TdcController::setTdcMeta(const Request& request) {
	mDevice->setTdcMeta(request.inputs().at(0));
	return {
		name(),
		"setTdcMeta",
		json::array(),
		true,
	};
}
