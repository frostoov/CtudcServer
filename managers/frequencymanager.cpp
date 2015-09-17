#include "frequencymanager.hpp"


namespace caen {

FrequencyManager::FrequencyManager(ModulePtr module, const ChannelConfig& config,
                                   const Microseconds& time)
    : ProcessManager(module, config),
      mDataValid(false),
      mTotalTime(Microseconds::zero()),
      mMsrTime(time) {}

bool FrequencyManager::isDataValid() {
    return mDataValid;
}

const TrekFrequency&FrequencyManager::frequency() const {
    return mFrequency;
}

bool FrequencyManager::init() {
    if(!ProcessManager::init())
        return false;
    else {
        mFrequency.clear();
        setBkpSettings(mTdcModule->settings());
        mTdcModule->setTriggerMode(false);
        if(!mTdcModule->settings().triggerMode()) {
            mTotalTime = Microseconds::zero();
            mDataValid = false;
            mTdcModule->setBlocked(true);
            return true;
        } else {
            returnSettings();
            return false;
        }
    }
}

void FrequencyManager::shutDown() {
    calculateFrequency(totalTime());
    mDataValid = true;
    returnSettings();
    mTdcModule->setBlocked(false);
}

void FrequencyManager::workerFunc() {
    auto buffer = mTdcModule->readWithClear(mMsrTime);
    handleData(buffer);
    mTotalTime += mMsrTime;
}

void FrequencyManager::handleData(WordVector& data) {
    for(auto word : data) {
        if(isMeasurement(word)) {
            auto channel = getChannel(word);
            if(checkChannel(channel)) {
                const auto& channelConfig = getChannelCongruence(channel);
                if(mFrequency.count(channelConfig.getChamber()) == 0)
                    mFrequency.insert({channelConfig.getChamber(), ChamberFrequency{{0, 0, 0, 0}} });
                ++mFrequency.at(channelConfig.getChamber()).at(channelConfig.getWire());
            }
        }
    }
}

void FrequencyManager::calculateFrequency(double time) {
    if(time != 0) {
        for(auto& chamFreq : mFrequency)
            for(auto& wireData : chamFreq.second)
                wireData /= time;
    } else
        mFrequency.clear();
}

void FrequencyManager::setFreqValid(bool flag) {
    mDataValid = flag;
}

FrequencyManager::Microseconds FrequencyManager::msrTime() const {
    return mMsrTime;
}

double FrequencyManager::totalTime() const {
    return double(mTotalTime.count()) / 1000000;
}


} //caen
