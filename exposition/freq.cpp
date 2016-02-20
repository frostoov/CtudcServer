#include "freq.hpp"

#include <thread>

using std::chrono::microseconds;
using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::runtime_error;
using std::vector;

FreqHandler::FreqHandler(ModulePtr module,
                         const ChannelConfig& config,
                         microseconds delay)
	: mDevice(module),
	  mConfig(config),
	  mDelay(delay),
	  mActive(false) { }

void FreqHandler::run() {
	if(mActive == true)
		throw runtime_error("FreqHandler::run FreqHandler already running");
	mTotalTime = microseconds::zero();
	mHitCount.clear();
	mActive = true;
	vector<Tdc::Hit> buffer;
	while(mActive) {
		mDevice->clear();
		auto startPoint = system_clock::now();
		std::this_thread::sleep_for(mDelay);
		mDevice->readRaw(buffer);
		mTotalTime += duration_cast<microseconds>(system_clock::now() - startPoint);
		handleBuffer(buffer);
	}
}

void FreqHandler::stop() {
	mActive = false;
}

FreqHandler::TrekFreq FreqHandler::freq() const {
	if(mActive)
		throw runtime_error("FreqHandler::freq FreqHandler running");
	TrekFreq freq;
	auto dur = double(mTotalTime.count()) / 1000000;
	for(auto chamPair : mHitCount) {
		auto& w = chamPair.second;
		freq.emplace(
		    chamPair.first,
		ChamberFreq{{unsigned(w[0] / dur), unsigned(w[1] / dur), unsigned(w[2] / dur), unsigned(w[3] / dur)}}
		);
	}
	return freq;
}

void FreqHandler::handleBuffer(const vector<Tdc::Hit>& buffer) {
	for(auto& hit : buffer) {
		if(mConfig.count(hit.channel) == 0)
			continue;
		auto& conf = mConfig.at(hit.channel);
		if(mHitCount.count(conf.chamber) == 0)
			mHitCount.emplace(conf.chamber, ChamberFreq{{0, 0, 0, 0}});
		++mHitCount.at(conf.chamber).at(conf.wire);
	}
}
