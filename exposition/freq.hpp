#pragma once

#include "process.hpp"
#include "channelconfig.hpp"
#include "tdc/caenv2718.hpp"

#include <chrono>
#include <array>
#include <atomic>
#include <unordered_map>
#include <memory>

class FreqHandler : public Process {
	using ModulePtr   = std::shared_ptr<CaenV2718>;
public:
	using ChamberFreq  = std::array<unsigned, 4>;
	using TrekFreq     = std::unordered_map<unsigned, ChamberFreq>;
public:
	FreqHandler(ModulePtr module,
	            const ChannelConfig& config,
	            std::chrono::microseconds delay);
	void run() override;
	void stop() override;
	TrekFreq freq() const;
protected:
	void handleBuffer(const std::vector<Tdc::Hit>& buffer);
private:
	ModulePtr                 mDevice;
	ChannelConfig             mConfig;
	TrekFreq                  mHitCount;
	std::chrono::microseconds mDelay;
	std::chrono::microseconds mTotalTime;
	std::atomic_bool          mActive;
};
