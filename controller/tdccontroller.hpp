#pragma once

#include "tdc/caenv2718.hpp"

#include <trek/net/controller.hpp>

class TdcController : public trek::net::Controller {
	using Module = CaenV2718;
	using ModulePtr = std::shared_ptr<Module>;
public:
	TdcController(const std::string& name, const ModulePtr& module);
protected:
	Methods createMethods();

	trek::net::Response open(const trek::net::Request& request);
	trek::net::Response close(const trek::net::Request& request);
	trek::net::Response isOpen(const trek::net::Request& request);

	trek::net::Response clear(const trek::net::Request &request);
	trek::net::Response reset(const trek::net::Request &request);
	trek::net::Response stat(const trek::net::Request& request);
	trek::net::Response ctrl(const trek::net::Request& request);
	trek::net::Response setMode(const trek::net::Request& request);
	trek::net::Response setWindowWidth(const trek::net::Request& request);
	trek::net::Response setWindowOffset(const trek::net::Request& request);
	trek::net::Response setEdgeDetection(const trek::net::Request& request);
	trek::net::Response setLsb(const trek::net::Request& request);
	trek::net::Response setCtrl(const trek::net::Request& request);
	trek::net::Response setTdcMeta(const trek::net::Request& request);
	trek::net::Response settings(const trek::net::Request& request);
private:
	ModulePtr mDevice;
};
