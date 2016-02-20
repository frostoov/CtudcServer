#pragma once

#include <trek/net/controller.hpp>
#include <trek/common/callback.hpp>
#include <future>

#include "tdc/caenv2718.hpp"
#include "exposition/channelconfig.hpp"
#include "exposition/process.hpp"
#include "exposition/exposition.hpp"
#include "exposition/freq.hpp"

class ProcessController : public trek::net::Controller {
	using ProcessPtr = std::unique_ptr<Process>;
	using ModulePtr  = std::shared_ptr<CaenV2718>;
public:
	ProcessController(const std::string& name,
	                  const ModulePtr& module,
	                  const Exposition::Settings& settings,
	                  const ChannelConfig& config);
	~ProcessController();
	const trek::Callback<void(unsigned)>& onNewRun();
protected:
	Methods createMethods();
	trek::net::Response getType(const trek::net::Request& request);
	trek::net::Response getRun(const trek::net::Request& request);
	trek::net::Response startRead(const trek::net::Request& request);
	trek::net::Response stopRead(const trek::net::Request& request);
	trek::net::Response startFreq(const trek::net::Request& request);
	trek::net::Response stopFreq(const trek::net::Request& request);
	trek::net::Response triggerCount(const trek::net::Request& request) const;
	trek::net::Response packageCount(const trek::net::Request& request) const;
	trek::net::Response duration(const trek::net::Request& request) const;
	bool isReadManager(const ProcessPtr& process) const;
	ProcessPtr createReadManager(const trek::net::Request& request) const;

	std::string getProcessType(const ProcessPtr& process) const;
	static nlohmann::json::array_t createFreqs(const FreqHandler::TrekFreq& freq);
private:
	ProcessPtr    mProcess;
	ModulePtr     mDevice;
	ChannelConfig mChannelConfig;

	Exposition::Settings         mSettings;
	trek::Callback<void(unsigned)> mOnNewRun;
	std::future<void> mFuture;
};
