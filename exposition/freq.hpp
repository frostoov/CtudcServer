#pragma once

#include "process.hpp"
#include "channelconfig.hpp"
#include "tdc/caenv2718.hpp"

#include <chrono>
#include <array>
#include <atomic>
#include <unordered_map>
#include <memory>

using ChamberFreq  = std::array<uintmax_t, 4>;
using TrekFreq     = std::unordered_map<unsigned, ChamberFreq>;

class FreqHandler : public Process {
	using SystemClock = std::chrono::system_clock;
	using ModulePtr   = std::shared_ptr<CaenV2718>;
public:
	FreqHandler(ModulePtr module,
	            const ChannelConfig& config,
	            std::chrono::microseconds delay);
	void run() override;
	void stop() override;
	bool running() override;
	std::chrono::milliseconds duration() const override;

	const TrekFreq& hitCount() const;
	std::chrono::microseconds cleanTime() const;
protected:
	void handleBuffer(const std::vector<Tdc::Hit>& buffer);
private:
	ModulePtr                 mDevice;
	ChannelConfig             mConfig;
	TrekFreq                  mHitCount;
	std::chrono::microseconds mDelay;
	std::chrono::microseconds mTotalTime;
	SystemClock::time_point   mStartPoint;
	std::atomic_bool          mActive;
};
