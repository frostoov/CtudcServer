#include "frequencymanager.hpp"

using cpp::Select;

namespace caen {

FrequencyManager::FrequencyManager(ModulePtr module, const ChannelConfig& config,
								   const Microseconds& time)
	: ProcessManager(module, config),
	  mDataValid(false),
	  mTotalMsrTime(Microseconds::zero()),
	  mMsrTime(time) {}

bool FrequencyManager::start() {
	mFrequency.clear();
	setBkpSettings(mTdcModule->getSettings());
	mTdcModule->setTriggerMode(false);
	if(!mTdcModule->getSettings().getTriggerMode())
		return ProcessManager::start();
	else {
		returnSettings();
		return false;
	}
}

void FrequencyManager::workerLoop() {
	mDataValid = false;
	WordVector buff;

	mTotalMsrTime = Microseconds::zero();
	while (isActive()) {
		measureFrequency(buff, mMsrTime);
		Select().recv(mStopChannel, [this](bool) {
			calculateFrequency(getTotalTime());
			mDataValid = true;
			returnSettings();
			setActive(false);
		});
	}
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

void FrequencyManager::measureFrequency(WordVector& buffer, const Microseconds& time) {
	mTdcModule->readBlock(buffer, time);
	handleData(buffer);
	buffer.clear();
	mTotalMsrTime += time;
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
