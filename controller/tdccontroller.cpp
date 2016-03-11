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
		{"open",                  [&](const Request & query) { return this->open(query); } },
		{"close",                 [&](const Request & query) { return this->close(query); } },
		{"isOpen",                [&](const Request & query) { return this->isOpen(query); } },
		{"clear",                 [&](const Request & query) { return this->clear(query); } },
		{"reset",                 [&](const Request & query) { return this->reset(query); } },
		{"stat",                  [&](const Request & query) { return this->stat(query); } },
		{"ctrl",                  [&](const Request & query) { return this->ctrl(query); } },
		{"tdcMeta",               [&](const Request & query) { return this->tdcMeta(query); } },
		{"setMode",	              [&](const Request & query) { return this->setMode(query); } },
		{"mode",                  [&](const Request & query) { return this->mode(query); } },
		{"setWindowWidth",        [&](const Request & query) { return this->setWindowWidth(query); } },
		{"setWindowOffset",       [&](const Request & query) { return this->setWindowOffset(query); } },
		{"setEdgeDetection",      [&](const Request & query) { return this->setEdgeDetection(query); } },
		{"setLsb",                [&](const Request & query) { return this->setLsb(query);  } },
		{"setCtrl",               [&](const Request & query) { return this->setCtrl(query); } },
		{"setTdcMeta",            [&](const Request & query) { return this->setTdcMeta(query); } },
		{"settings",              [&](const Request & query) { return this->settings(query); } },
		{"updateSettings",        [&](const Request & query) { return this->updateSettings(query); } },
	};
}

Response TdcController::open(const Request& request) {
	mDevice->open();
	return { name(), __func__ };
}

Response TdcController::close(const Request& request) {
	mDevice->close();
	return { name(), __func__ };
}

Response TdcController::isOpen(const Request& request) {
	return { name(), __func__, {mDevice->isOpen()} };
}

Response TdcController::clear(const Request& request) {
	mDevice->clear();
	return { name(), __func__ };
}

Response TdcController::reset(const Request& request) {
	mDevice->reset();
	return { name(), __func__ };
}

Response TdcController::setMode(const Request& request) {
	auto mode = request.inputs.at(0).get<int>();
	mDevice->setMode(Tdc::Mode(mode));
	return { name(), __func__ };
}

Response TdcController::setWindowWidth(const Request& request) {
	mDevice->setWindowWidth(request.inputs.at(0));
	return { name(), __func__ };
}

Response TdcController::setWindowOffset(const Request& request) {
	mDevice->setWindowOffset(request.inputs.at(0));
	return { name(), __func__ };
}

Response TdcController::setEdgeDetection(const Request& request) {
	auto ed = request.inputs.at(0).get<int>();
	mDevice->setEdgeDetection(Tdc::EdgeDetection(ed));
	return { name(), __func__ };
}

Response TdcController::setLsb(const Request& request) {
	mDevice->setLsb(request.inputs.at(0));
	return { name(), __func__ };
}

Response TdcController::setCtrl(const Request& request) {
	mDevice->setCtrl(request.inputs.at(0));
	return { name(), __func__ };
}

Response TdcController::stat(const Request& request) {
	return { name(), __func__, {mDevice->stat()} };
}

Response TdcController::ctrl(const Request& request) {
	return { name(), __func__, {mDevice->ctrl()} };
}

Response TdcController::tdcMeta(const Request& request) {
	return { name(), __func__, {mDevice->tdcMeta()} };
}

Response TdcController::mode(const Request& request) {
	return { name(), __func__, {int(mDevice->mode())} };
}

Response TdcController::updateSettings(const Request& request) {
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

Response TdcController::settings(const Request& request) {
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

Response TdcController::setTdcMeta(const Request& request) {
	mDevice->setTdcMeta(request.inputs.at(0));
	return { name(), __func__ };
}
