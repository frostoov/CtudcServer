#pragma once

#include <trek/net/controller.hpp>
#include <trek/common/callback.hpp>

#include "tdc/tdc.hpp"
#include "managers/channelconfig.hpp"
#include "managers/processmanager.hpp"
#include "managers/readmanager.hpp"

class ProcessController : public trek::net::Controller {
	using ProcessPtr = std::unique_ptr<ProcessManager>;
	using ModulePtr  = std::shared_ptr<Tdc>;
public:
	ProcessController(const std::string& name,
                      const ModulePtr& module,
					  const ReadManager::Settings& settings,
                      const ChannelConfig& config);
	const trek::Callback<void(unsigned)>& onNewRun();
protected:
	Methods createMethods();
	trek::net::Response getType(const trek::net::Request& request);
	trek::net::Response getRun(const trek::net::Request& request);
	trek::net::Response startRead(const trek::net::Request& request);
	trek::net::Response stopRead(const trek::net::Request& request);
	trek::net::Response getTriggerFrequency(const trek::net::Request& request) const;
	trek::net::Response getPackageFrequency(const trek::net::Request& request) const;
	bool isReadManager(const ProcessPtr& process) const;
	ProcessPtr createReadManager(const trek::net::Request& request) const;

	std::string getProcessType(const ProcessPtr &process) const;
private:
	ProcessPtr    mProcess;
	ModulePtr     mDevice;
	ChannelConfig mChannelConfig;

	ReadManager::Settings         mSettings;
	trek::Callback<void(unsigned)> mOnNewRun;
};
