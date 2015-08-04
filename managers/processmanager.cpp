#include "processmanager.hpp"


namespace caen {

ProcessManager::ProcessManager(ModulePtr module, const ChannelConfig& config)
	: mTdcModule(module),
	  mConfig(config),
	  mIsActive(false),
	  mIsInLoop(false),
	  mIsProcDone(true) {}

void ProcessManager::setBkpSettings(const tdcdata::Settings& settings) {
	mBkpSettings = settings;
}

void ProcessManager::returnSettings() {
	if(mBkpSettings != mTdcModule->getSettings())
		mTdcModule->setSettings(mBkpSettings);
}

bool ProcessManager::start() {
	if(!mTdcModule || !mTdcModule->isInit() || mIsActive)
		return false;
	setProcDone(false);
	mIsActive = true;
	std::thread workerLoopThread(&ProcessManager::workerLoop, this);
	workerLoopThread.detach();
	return true;
}

void ProcessManager::stop() {
	mIsActive = false;
	while(mIsInLoop);
}

ProcessManager::TDCRecordList ProcessManager::handleBuffer(const WordVector& buffer) {
	TDCRecordList chunkOfBuffers;
	WordVector tempBuffer;
	bool header = false;
	for(auto word : buffer) {
		if(isMeasurement(word))
			tempBuffer.push_back(word);
		else if( isGlobalHeader(word) && header == false )
			header = true;
		else if( isGlobalTrailer(word) && header == true) {
			header = false;
			chunkOfBuffers.push_back(convertEvent(tempBuffer));
			tempBuffer.clear();
		}
	}
	return chunkOfBuffers;
}

uint32_t ProcessManager::convertWord(uint32_t word) {
	auto channel = getChannel(word);
	if(checkChannel(channel)) {
		const auto& config = getChannelCongruence(channel);
		word &= 0x7FFFF;
		word |= config.getChamber() << 19;
		word |= config.getWire()   << 24;
		return word;
	} else
		throw std::runtime_error("ModuleHandler::convertWord: no config for channel");
}

WordVector& ProcessManager::convertEvent(WordVector& eventWords) {
	for(auto& word : eventWords) {
		if(isMeasurement(word))
			word = convertWord(word);
		else
			throw std::runtime_error("ModuleHandler::convertEvent: non measurement word");
	}
	return eventWords;
}

bool caen::ProcessManager::checkChannel(size_t ch) const {
	return static_cast<bool>(mConfig.count(ch));
}

const ChannelCongruence& ProcessManager::getChannelCongruence(size_t ch) {
	return mConfig.at(ch);
}

} //caen
