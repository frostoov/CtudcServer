#include <fstream>
#include <thread>
#include <tdcdata/serialization.hpp>


#include "net/nettools.hpp"
#include "ctudcreadmanager.hpp"

using std::string;
using std::ofstream;
using std::ostream;
using std::istream;
using std::exception;
using std::chrono::milliseconds;
using std::chrono::microseconds;

using tdcdata::DataSetType;
using tdcdata::CtudcRecord;
using tdcdata::DecorPackage;
using tdcdata::NevodPackage;

using cpp::Select;

namespace caen {

CtudcReadManager::CtudcReadManager(ModulePtr module, const string& path, size_t eventNum,
								   const ChannelConfig& config,  const NetInfo& netInfo)
	: ReadManager(module, path, eventNum, config),
	  mDecorReciever(netInfo.decorIP, netInfo.decorPort),
	  mNevodReciever(netInfo.nevodIP, netInfo.nevodPort),
	  mIsHandling(false) {
	setFileType(DataSetType::CTUDC);
	mDecorReciever.setCallback([this](char* data, size_t size) {
		this->handleDecorPackage(data, size);
	});
	mNevodReciever.setCallback([this](char* data, size_t size) {
		this->handleNevodPackage(data, size);
	});
}

CtudcReadManager::~CtudcReadManager() {
	closeStream(mStream);
}

void CtudcReadManager::workerLoop() {
	WordVector buffer;

	mTdcModule->softwareClear();
	mDecorPackages.clear();
	mNevodPackages.clear();

	mDecorReciever.start();
	mNevodReciever.start();

	while(isActive()) {
		//Блокируем поток и ожидаем DecorPackage
		waitForDecorPackage();
		if(isActive()) {
			auto startTime = SystemClock::now();
			buffer.clear();
			mTdcModule->readBlock(buffer);
			//Получили DecorPackage, блокируем поток и ожидаем NevodPackage
			waitForNevodPackage(startTime);

			handleDataPackages(buffer);
		}
		Select().recv(mStopChannel, [this](bool) {
			closeStream(mStream);
			returnSettings();
			setActive(false);
		}).tryOnce();
	}
}

void CtudcReadManager::stop() {
	mDecorReciever.stop();
	mNevodReciever.stop();
	mDecorBlocker.unblock();
	ReadManager::stop();
}

void CtudcReadManager::handleDataPackages(WordVector& tdcData) {
	mIsHandling = true;
	auto events = handleBuffer(tdcData);
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
	if(!mDecorPackages.empty()) {
		record.setDecorPacakge(mDecorPackages.front());
		mDecorPackages.pop_front();
	}
	if(!mNevodPackages.empty()) {
		record.setNevodPackage(mNevodPackages.front());
		mNevodPackages.pop_front();
	}
	writeCtudcRecord(record);
	mIsHandling = false;
}

void CtudcReadManager::handleDecorPackage(char* data, size_t size) {
	if(!isHandling() && mDecorPackages.empty()) {
		if(verifyDecorPackage(data, size)) {
			DecorPackage package;
			membuf  tempBuffer(data, size);
			istream stream(&tempBuffer);
			package.deserialize(stream);

			mDecorPackages.push_back(package);

			mDecorBlocker.unblock();
		}
	}
}

void CtudcReadManager::handleNevodPackage(char* data, size_t size) {
	if(!isHandling() && mNevodPackages.empty()) {
		if(verifyNevodPackage(data, size)) {
			NevodPackage package;
			membuf tempBuffer(data, size);
			istream stream(&tempBuffer);
			package.deserialize(stream);

			mNevodPackages.push_back(package);
		}
	}
}

void CtudcReadManager::waitForDecorPackage() {
	if(mDecorPackages.empty())
		mDecorBlocker.block();
	/* Блокировку сниммет Callback(CtudcReadManager::handleDecorPackage)
	 * при получении корретного DecorPackage
	 */
}

void CtudcReadManager::waitForNevodPackage(SystemClock::time_point startTime) {
	milliseconds awaitTime(5);
	while(SystemClock::now() - startTime < awaitTime) {
		if(mNevodPackages.empty() == false)
			return;
	}
}

void CtudcReadManager::writeCtudcRecord(const CtudcRecord& record) {
	if(!mStream.is_open() || needNewStream())
		openStream(mStream);

	::serialize(mStream, record);
	increaseEventCount();
}

} //caen
