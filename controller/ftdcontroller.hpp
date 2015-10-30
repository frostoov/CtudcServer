#pragma once
#include "ftd/ftdmodule.hpp"

#include <trek/net/controller.hpp>

class FtdController : public trek::net::Controller {
	using ModulePtr = std::shared_ptr<ftdi::Module>;
public:
	FtdController(const std::string& name, const ModulePtr& module);
protected:
	Methods createMethods();
	trek::net::Response init(const trek::net::Request& request);
	trek::net::Response close(const trek::net::Request& request);
	trek::net::Response isInit(const trek::net::Request& request);
	trek::net::Response setCodes(const trek::net::Request& request);
private:
	ModulePtr mDevice;
};
