#include <fstream>
#include <tdcdata/serialization.hpp>

#include "net/nettools.hpp"
#include "ctudcreadmanager.hpp"

using std::string;
using std::ofstream;
using std::ostream;
using std::istream;
using std::exception;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::make_unique;
using std::vector;
using std::chrono::duration_cast;

using tdcdata::DataSetType;
using tdcdata::CtudcRecord;
using tdcdata::NevodPackage;


namespace caen {


CtudcReadManager::CtudcReadManager(ModulePtr module, const string& path, size_t eventNum,
								   const ChannelConfig& config,  const NetInfo& netInfo)
	: ReadManager(module, path, eventNum, config),
	  mNevodReciever(netInfo.nevodIP, netInfo.nevodPort),
	  mNetIsActive(true) {
	setFileType(DataSetType::CTUDC);
}

bool CtudcReadManager::start() {
	if(!init()) {
		return false;
	} else {
		mNevodReciever.setCallback([this](PackageReceiver::ByteVector& buffer) {
			handleNevodPackage(buffer);
			mBuffer.clear();
			mTdcModule->readBlock(mBuffer);
			handleDataPackages(mBuffer);
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
	return double(mTriggerCount)/duration_cast<seconds>(SystemClock::now() - mStartPoint).count();
}

double CtudcReadManager::getPackageFrequency() const {
	return double(mPackageCount)/duration_cast<seconds>(SystemClock::now() - mStartPoint).count();
}

bool CtudcReadManager::init() {
	if(!ReadManager::init())
		return false;
	else {
		mTriggerCount = 0;
		mPackageCount = 0;
		mStartPoint = std::chrono::high_resolution_clock::now();
		return true;
	}
}

void CtudcReadManager::workerFunc() {
	throw std::logic_error("CtudcReadManager::workerFunc: call not allowed");
}

void CtudcReadManager::handleDataPackages(WordVector& tdcData) {
	auto events = handleBuffer(tdcData);
	mTriggerCount += events.size();
	auto timePoint = SystemClock::now();
	while(events.size() > 1) {
		CtudcRecord record(getEventCount(), timePoint);
		record.setTdcData(events.front().getData());
		events.pop_front();
		writeCtudcRecord(record);
	}
	CtudcRecord record(getEventCount(), timePoint);
	if(!events.empty()) {
		record.setTdcData(events.front().getData());
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
		deserialize(stream, *mNevodPackage);
		++mPackageCount;
	}
}

void CtudcReadManager::writeCtudcRecord(const CtudcRecord& record) {
	if(!mStream.is_open() || needNewStream())
		openStream(mStream);

	serialize(mStream, record);
	increaseEventCount();
}

} //caen
