#include "freq.hpp"

#include <thread>
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::duration_cast;
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
	  mActive(false) {
	if(!mDevice->isOpen())
		throw runtime_error("FreqHandler::FreqHandler device is not open");
}

void FreqHandler::run() {
	if(mActive == true)
		throw runtime_error("FreqHandler::run FreqHandler already running");
    mStartPoint = SystemClock::now();
	mTotalTime = microseconds::zero();
	mHitCount.clear();
	mActive = true;
	vector<Tdc::Hit> buffer;
	while(mActive.load()) {
		mDevice->clear();
		auto startPoint = system_clock::now();
		std::this_thread::sleep_for(mDelay);
		mDevice->readRaw(buffer);
        auto delta = duration_cast<microseconds>(system_clock::now() - startPoint);
		mTotalTime += delta;
		handleBuffer(buffer);
	}
}

void FreqHandler::stop() {
	mActive.store(false);
}

bool FreqHandler::running() {
    return mActive.load();
}

milliseconds FreqHandler::duration() const {
    return duration_cast<milliseconds>(SystemClock::now() - mStartPoint);
}

microseconds FreqHandler::cleanTime() const {
    return mTotalTime;
}

const TrekFreq& FreqHandler::hitCount() const {
    return mHitCount;
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
