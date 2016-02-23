#pragma once
#include "voltage/amplifier.hpp"

#include <trek/net/controller.hpp>
#include <json.hpp>

class VoltageController : public trek::net::Controller {
	using Module = Amplifier;
	using ModulePtr = std::shared_ptr<Module>;
public:
	struct Config {
		int cell2;
		int cell12;

		nlohmann::json marshal() const;
		void unMarhsal(const nlohmann::json& json);
	};
public:
	VoltageController(const std::string& name, const ModulePtr& module, const Config& config);
protected:
    Methods createMethods();

    trek::net::Response open(const trek::net::Request& request);
	trek::net::Response close(const trek::net::Request& request);
	trek::net::Response isOpen(const trek::net::Request& request);

	trek::net::Response turnOn2(const trek::net::Request& request);
	trek::net::Response turnOff2(const trek::net::Request& request);
    trek::net::Response setVoltage2(const trek::net::Request& request);
	trek::net::Response voltage2(const trek::net::Request& request);

    trek::net::Response turnOn12(const trek::net::Request& request);
	trek::net::Response turnOff12(const trek::net::Request& request);
	trek::net::Response voltage12(const trek::net::Request& request);
private:
    ModulePtr mDevice;
    Config    mConfig;
};
