#include "processmanager.hpp"

using std::to_string;

namespace caen {

ProcessManager::ProcessManager (ModulePtr module, const ChannelConfig& config)
	: mTdcModule (module),
	  mConfig (config) { }

bool ProcessManager::init() {
	if ( mTdcModule && mTdcModule->isInit() )
	{ return true; }
	else
	{ return false; }
}

void ProcessManager::setBkpSettings (const trekdata::Settings& settings) {
	mBkpSettings = settings;
}

void ProcessManager::returnSettings() {
	if (mBkpSettings != mTdcModule->getSettings() )
	{ mTdcModule->setSettings (mBkpSettings); }
}

ProcessManager::TdcRecordList ProcessManager::handleBuffer (const WordVector& buffer) {
	TdcRecordList chunkOfBuffers;
	WordVector tempBuffer;
	bool header = false;
	for (auto word : buffer) {
		if (isMeasurement (word) )
		{ tempBuffer.push_back (word); }
		else if ( isGlobalHeader (word) && header == false )
		{ header = true; }
		else if ( isGlobalTrailer (word) && header == true) {
			header = false;
			chunkOfBuffers.push_back (convertEvent (tempBuffer) );
			tempBuffer.clear();
		}
	}
	return chunkOfBuffers;
}

uint32_t ProcessManager::convertWord (uint32_t word) {
	auto channel = getChannel (word);
	if (checkChannel (channel) ) {
		const auto& config = getChannelCongruence (channel);
		word &= 0x7FFFF;
		word |= config.getWire()    << 19;
		word |= config.getChamber() << 24;
		return word;
	} else
		throw std::runtime_error ("ProcessManager::convertWord: no config for channel: " +
		                          to_string (channel) );
}

WordVector& ProcessManager::convertEvent (WordVector& eventWords) {
	for (auto& word : eventWords) {
		if (isMeasurement (word) )
		{ word = convertWord (word); }
		else
		{ throw std::runtime_error ("ProcessManager::convertEvent: non measurement word"); }
	}
	return eventWords;
}

bool caen::ProcessManager::checkChannel (uint32_t channel) const {
	return mConfig.count (channel) != 0;
}

const ChannelCongruence& ProcessManager::getChannelCongruence (size_t ch) {
	return mConfig.at (ch);
}

} //caen
