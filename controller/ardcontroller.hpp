#pragma once

#include "voltage/voltage.hpp"

#include <trek/net/controller.hpp>

class ArdController : public trek::net::Controller {
	using Module = Voltage;
	using ModulePtr = std::shared_ptr<Module>;
public:
	ArdController(const std::string& name, const ModulePtr& module);
protected:
	Methods createMethods();

	trek::net::Response open(const trek::net::Request& request);
	trek::net::Response close(const trek::net::Request& request);
	trek::net::Response isOpen(const trek::net::Request& request);

    trek::net::Response turnOn(const trek::net::Request& request);
    trek::net::Response turnOff(const trek::net::Request& request);
    trek::net::Response voltage(const trek::net::Request& request);
private:
	ModulePtr mDevice;
};
