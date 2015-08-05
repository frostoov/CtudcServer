#include "frequencymanager.hpp"


namespace caen {

FrequencyManager::FrequencyManager(ModulePtr module, const ChannelConfig& config,
                                   const Microseconds& time)
	: ProcessManager(module, config),
	  mDataValid(false),
	  mTotalMsrTime(Microseconds::zero()),
	  mMsrTime(time) {}

bool FrequencyManager::init() {
	if(!ProcessManager::init())
		return false;
	else {
		mBuffer.clear();
		mFrequency.clear();
		setBkpSettings(mTdcModule->getSettings());
		mTdcModule->setTriggerMode(false);
		if(!mTdcModule->getSettings().getTriggerMode()) {
			mTotalMsrTime = Microseconds::zero();
			mDataValid = false;
			return ProcessManager::start();
		} else {
			returnSettings();
			return false;
		}
	}
}

void FrequencyManager::shutDown() { }

void FrequencyManager::flush() {
	calculateFrequency(getTotalTime());
	mDataValid = true;
	returnSettings();
}

void FrequencyManager::workerFunc() {
	mTdcModule->readBlock(mBuffer, mMsrTime);
	handleData(mBuffer);
	mBuffer.clear();
	mTotalMsrTime += mMsrTime;
}

void FrequencyManager::handleData(WordVector& data) {
	for (auto word : data) {
		if (isMeasurement(word)) {
			auto channel = getChannel(word);
			if (checkChannel(channel)) {
				const auto& channelConfig = getChannelCongruence(channel);
				if( mFrequency.count(channelConfig.getChamber()) == 0)
					mFrequency.insert( {channelConfig.getChamber(), ChamberFrequency{{0, 0, 0, 0}} } );
				++mFrequency.at(channelConfig.getChamber()).at(channelConfig.getWire());
			}
		}
	}
}

void FrequencyManager::calculateFrequency(double time) {
	if(time != 0) {
		for (auto& chamFreq : mFrequency)
			for (auto& wireData : chamFreq.second)
				wireData /= time;
	} else
		mFrequency.clear();
}


} //caen
