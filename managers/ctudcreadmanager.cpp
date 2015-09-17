#include <trek/common/serialization.hpp>
#include <trek/common/applog.hpp>

#include <chrono>

#include "net/nettools.hpp"
#include "ctudcreadmanager.hpp"

using std::endl;

using std::string;
using std::istream;
using std::exception;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::make_unique;
using std::vector;
using std::chrono::system_clock;
using std::chrono::duration_cast;

using trek::Log;

using trek::data::DataSetType;
using trek::data::CtudcRecord;
using trek::data::NevodPackage;


namespace caen {

CtudcReadManager::CtudcReadManager(ModulePtr module,
                                   const ChannelConfig& config,
                                   const string& path,
                                   uintmax_t eventsPerFile,
                                   uint64_t numberOfRun,
                                   const NetInfo& netInfo)
    : ReadManager(module, config, path, eventsPerFile, numberOfRun),
      mNevodReciever(netInfo.nevodIP, netInfo.nevodPort) {
    setFileType(DataSetType::Ctudc);
}

bool CtudcReadManager::start() {
    if(!init())
        return false;
    else {
        mNevodReciever.setCallback([this](PackageReceiver::ByteVector& nevodBuffer) {
            try {
                handleNevodPackage(nevodBuffer);
                auto tdcBuffer = mTdcModule->read();
                handleDataPackages(tdcBuffer);
            } catch(const std::exception& e) {
                Log::instance() << "CtudcReadManager: Failed handle buffer" << std::endl;
            }
        });
        return startThread([this]() {
            mNevodReciever.start();
        });
    }
}

void CtudcReadManager::stop() {
    mNevodReciever.stop();
    mNevodReciever.resetCallback();
    resetThread();
    shutDown();
}

double CtudcReadManager::getTriggerFrequency() const {
    return double (mTriggerCount) / duration_cast<seconds> (SystemClock::now() - mStartPoint).count();
}

double CtudcReadManager::getPackageFrequency() const {
    return double (mPackageCount) / duration_cast<seconds> (SystemClock::now() - mStartPoint).count();
}

bool CtudcReadManager::init() {
    if(!ReadManager::init())
        return false;
    else {
        resetPackageCount();
        resetTriggerCount();
        mStartPoint = std::chrono::high_resolution_clock::now();
        return true;
    }
}

void CtudcReadManager::workerFunc() {
    throw std::logic_error("CtudcReadManager::workerFunc: call not allowed");
}

void CtudcReadManager::increasePackageCount() {
    ++mPackageCount;
}

void CtudcReadManager::increaseTriggerCount(uintmax_t val) {
    mTriggerCount += val;
}

void CtudcReadManager::resetPackageCount() {
    mPackageCount = 0;
}

void CtudcReadManager::resetTriggerCount() {
    mTriggerCount = 0;
}

void CtudcReadManager::handleDataPackages(WordVector& tdcData) {
    auto events = handleBuffer(tdcData);
    increaseTriggerCount(events.size());
    auto timePoint = SystemClock::now();
    while(events.size() > 1) {
        CtudcRecord record(getNumberOfRun(), getNumberOfRecord(), timePoint);
        record.setTdcRecord(events.front());
        events.pop_front();
        writeCtudcRecord(record);
    }
    CtudcRecord record(getNumberOfRun(), getNumberOfRecord(), timePoint);
    if(!events.empty()) {
        record.setTdcRecord(events.front());
        events.pop_front();
    }
    if(mNevodPackage) {
        record.setNevodPackage(*mNevodPackage);
        mNevodPackage.reset();
    }
    writeCtudcRecord(record);
}

void CtudcReadManager::handleNevodPackage(PackageReceiver::ByteVector& buffer) {
    if(verifyNevodPackage(buffer.data(), buffer.size())) {
        mNevodPackage = make_unique<NevodPackage>();
        membuf tempBuffer(buffer.data(), buffer.size());
        istream stream(&tempBuffer);
        trek::deserialize(stream, *mNevodPackage);
        increasePackageCount();
    }
}

void CtudcReadManager::writeCtudcRecord(const CtudcRecord& record) {
    if(!mStream.is_open() || needNewStream())
        openStream(mStream);

    trek::serialize(mStream, record);
    increaseRecordCount();
}

} //caen
