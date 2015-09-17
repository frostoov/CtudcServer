#ifndef FREQUENCYMANAGER_HPP
#define FREQUENCYMANAGER_HPP

#include "processmanager.hpp"

namespace caen {

using ChamberFrequency = std::array<uint32_t, 4>;
using TrekFrequency    = std::unordered_map<uintmax_t, ChamberFrequency>;

class FrequencyManager : public ProcessManager {
public:
    using Microseconds = std::chrono::microseconds;
    using Seconds = std::chrono::seconds;

    FrequencyManager(ModulePtr module, const ChannelConfig& config,
                     const Microseconds& time);
    bool isDataValid();
    const TrekFrequency& frequency() const;

protected:
    bool init() override;
    void shutDown() override;
    void workerFunc() override;
    void handleData(WordVector& data);
    void calculateFrequency(double time);
    void clearFreq();
    void setFreqValid(bool flag);

    Microseconds msrTime() const;
    double totalTime() const;

private:
    TrekFrequency mFrequency;
    bool mDataValid;
    Microseconds mTotalTime;
    Microseconds mMsrTime;
};

}

#endif  // FREQUENCYMANAGER_HPP
