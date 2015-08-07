#include <fstream>
#include <tdcdata/tdcrecord.hpp>
#include <tdcdata/serialization.hpp>
#include <boost/filesystem.hpp>

#include "readmanager.hpp"
#include "types.hpp"

using std::to_string;
using std::ofstream;
using std::ostream;
using std::string;
using std::exception;
using std::runtime_error;

using boost::filesystem::path;
using boost::filesystem::exists;
using boost::filesystem::is_directory;
using boost::filesystem::create_directory;

using tdcdata::TDCRecord;
using tdcdata::DataSetType;
using tdcdata::DataSetHeader;

namespace caen {

ReadManager::ReadManager(ModulePtr module, const string& dirName,
						 size_t eventsPerFile, const ChannelConfig& channelConfig)
	: ProcessManager(module, channelConfig),
	  mEventCount(0),
	  mFileCount(0),
	  mEventsPerFile(eventsPerFile),
	  mFileType(DataSetType::Simple) {
	auto dirPath = path(dirName);
	if(exists(dirPath)) {
		if(!is_directory(dirPath))
			throw std::runtime_error("Invalid dir path");
	} else
		create_directory(dirPath);
	mPath = dirName;
	mStream.exceptions(ofstream::badbit | ofstream::failbit);
}

bool ReadManager::init() {
	if(!ProcessManager::init())
		return false;
	else {
		mBuffer.clear();
		resetEventCount();
		resetFileCount();
		setBkpSettings(mTdcModule->getSettings());
		mTdcModule->setTriggerMode(true);
		if(mTdcModule->getSettings().getTriggerMode()) {
			mTdcModule->softwareClear();
			return true;
		} else {
			returnSettings();
			return false;
		}
	}
}

void ReadManager::shutDown() {
	returnSettings();
	closeStream(mStream);
}

void ReadManager::workerFunc() {
	mBuffer.clear();
	auto readSize = mTdcModule->readBlock(mBuffer);
	if(readSize) {
		auto events = handleBuffer(mBuffer);
		for(const auto& event : events)
			writeTdcRecord(event);
	}
}

void ReadManager::writeTdcRecord(const tdcdata::TDCRecord& event) {
	if(mStream.is_open() == false || needNewStream())
		openStream(mStream);
	::serialize(mStream, event);
	increaseEventCount();
}

bool ReadManager::needNewStream() {
	if(mEventCount % mEventsPerFile == 0 && mEventCount != 0)
		return true;
	else
		return false;
}


bool ReadManager::openStream(ofstream& stream) {
	if(stream.is_open())
		closeStream(stream);
	stream.open(mPath + "/NAD_" + to_string(mFileCount) + ".tds", ofstream::binary);

	DataSetHeader header(mFileType, 52015, mTdcModule->getSettings() );

	::serialize(stream, header);

	return true;
}

void ReadManager::closeStream(ofstream& stream) {
	if( stream.is_open()) {
		stream.close();
		increaseFileCount();
	}
}

}
//caen
