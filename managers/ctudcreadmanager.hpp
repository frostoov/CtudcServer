#ifndef URAGANREADMANAGER_HPP
#define URAGANREADMANAGER_HPP

#include <trekdata/ctudcrecord.hpp>

#include "net/packagereceiver.hpp"
#include "readmanager.hpp"

namespace caen {

class CtudcReadManager : public ReadManager {
protected:
    using NevodPkgPtr     = std::unique_ptr<trekdata::NevodPackage>;
    using SystemClock     = std::chrono::high_resolution_clock;
    using TimePoint       = SystemClock::time_point;
public:
    struct NetInfo {
        std::string nevodIP;
        uint16_t    nevodPort;
    };
public:
    CtudcReadManager(ModulePtr module,
                     const ChannelConfig& config,
                     const std::string& path,
                     uintmax_t eventsPerFile,
                     uint64_t numberOfRun,
                     const NetInfo& netInfo);
    bool start() override;
    void stop() override;
    double getTriggerFrequency() const;
    double getPackageFrequency() const;
protected:
    bool init() override;
    void workerFunc() override;

    void increasePackageCount();
    void increaseTriggerCount(uintmax_t val);
    void resetPackageCount();
    void resetTriggerCount();
    void handleDataPackages(WordVector& tdcData);
    void handleNevodPackage(PackageReceiver::ByteVector& buffer);
    void writeCtudcRecord(const trekdata::CtudcRecord& record);
private:
    PackageReceiver mNevodReciever;
    NevodPkgPtr mNevodPackage;

    uintmax_t mPackageCount;
    uintmax_t mTriggerCount;

    TimePoint mStartPoint;
    bool mNetIsActive;
};

} //caen

#endif // URAGANREADMANAGER_HPP
