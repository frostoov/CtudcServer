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
using std::make_unique;
using std::vector;

using tdcdata::DataSetType;
using tdcdata::CtudcRecord;
using tdcdata::DecorPackage;
using tdcdata::NevodPackage;


namespace caen {

CtudcReadManager::CtudcReadManager(ModulePtr module, const string& path, size_t eventNum,
                                   const ChannelConfig& config,  const NetInfo& netInfo)
	: ReadManager(module, path, eventNum, config),
	  mDecorReciever(netInfo.decorIP, netInfo.decorPort),
	  mNevodReciever(netInfo.nevodIP, netInfo.nevodPort),
	  mDecorChannel(mDecorReciever.getDataChannel()),
	  mNevodChannel(mNevodReciever.getDataChannel()) {
	setFileType(DataSetType::CTUDC);
}

bool CtudcReadManager::init() {
	if(!ReadManager::init())
		return false;
	else {
		mDecorReciever.start();
		mNevodReciever.start();
		return true;
	}
}

void CtudcReadManager::shutDown() {
	ReadManager::shutDown();
	mDecorReciever.stop();
	mNevodReciever.stop();
}

void CtudcReadManager::workerFunc() {
	//Блокируем поток и ожидаем DecorPackage
	waitForDecorPackage();
	auto startTime = SystemClock::now();
	mBuffer.clear();
	mTdcModule->readBlock(mBuffer);
	//Получили DecorPackage, блокируем поток и ожидаем NevodPackage
	waitForNevodPackage(startTime);

	handleDataPackages(mBuffer);
}

void CtudcReadManager::handleDataPackages(WordVector& tdcData) {
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
	if(mDecorPackage) {
		record.setDecorPacakge(*mDecorPackage);
		mDecorPackage.reset();
	}
	if(mNevodPackage) {
		record.setNevodPackage(*mNevodPackage);
		mNevodPackage.reset();
	}
	writeCtudcRecord(record);
}

void CtudcReadManager::handleDecorPackage(ByteVector&& buffer) {
	if(verifyDecorPackage(buffer.data(), buffer.size())) {
		mDecorPackage = make_unique<DecorPackage>();
		membuf  tempBuffer(buffer.data(), buffer.size());
		istream stream(&tempBuffer);
		deserialize(stream, *mDecorPackage);
	}
}

void CtudcReadManager::handleNevodPackage(ByteVector&& buffer) {
	if(verifyNevodPackage(buffer.data(), buffer.size())) {
		mNevodPackage = make_unique<NevodPackage>();
		membuf tempBuffer(buffer.data(), buffer.size());
		istream stream(&tempBuffer);
		deserialize(stream, *mNevodPackage);
	}
}

void CtudcReadManager::waitForDecorPackage() {
	if(mDecorChannel.isOpen()) {
		ByteVector data;
		if(mDecorChannel.recv(data))
			handleDecorPackage(std::move(data));
	}
}

void CtudcReadManager::waitForNevodPackage(SystemClock::time_point startTime) {
	if(!mNevodChannel.isOpen())
		return;
	milliseconds awaitTime(5);
	ByteVector tempBuffer;
	while(SystemClock::now() - startTime < awaitTime) {
		if(mNevodChannel.tryRecv(tempBuffer)) {
			handleNevodPackage(std::move(tempBuffer));
			break;
		}
	};
}

void CtudcReadManager::writeCtudcRecord(const CtudcRecord& record) {
	if(!mStream.is_open() || needNewStream())
		openStream(mStream);

	serialize(mStream, record);
	increaseEventCount();
}

} //caen
