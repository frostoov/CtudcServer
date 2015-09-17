#include <fstream>
#include <sstream>
#include <iomanip>

#include <trek/data/tdcrecord.hpp>
#include <trek/common/serialization.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include "readmanager.hpp"
#include "channelconfig.hpp"

using std::to_string;
using std::ofstream;
using std::ostream;
using std::string;
using std::exception;
using std::runtime_error;
using std::ostringstream;
using std::setfill;
using std::setw;

namespace fs = boost::filesystem;

using trek::data::TdcRecord;
using trek::data::DataSetType;
using trek::data::DataSetHeader;

namespace caen {

ReadManager::ReadManager(ModulePtr module,
                         const ChannelConfig& channelConfig,
                         const string& dirName,
                         uint64_t numberOfRun,
                         uintmax_t eventsPerFile)
    : ProcessManager(module, channelConfig),
      mPath(dirName),
      mNumberOfRun(numberOfRun),
      mNumberOfRecord(0),
      mNumberOfFiles(0),
      mEventsPerFile(eventsPerFile),
      mFileType(DataSetType::Simple) {
    fs::create_directories(fs::path(mPath));
    mStream.exceptions(ofstream::badbit | ofstream::failbit);
}

bool ReadManager::init() {
    if(!ProcessManager::init())
        return false;
    else {
        resetRecordCount();
        resetFileCount();
        setBkpSettings(mTdcModule->settings());
        mTdcModule->setTriggerMode(true);
        if(mTdcModule->settings().triggerMode()) {
            mTdcModule->softwareClear();
            mTdcModule->setBlocked(true);
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
    mTdcModule->setBlocked(false);
}

void ReadManager::workerFunc() {
    auto buffer = mTdcModule->read();
    if(!buffer.empty()) {
        auto events = handleBuffer(buffer);
        for(const auto& event : events)
            writeTdcRecord(event);
    }
}

void ReadManager::writeTdcRecord(const trek::data::TdcRecord& event) {
    if(mStream.is_open() == false || needNewStream())
        openStream(mStream);
    trek::serialize(mStream, event);
    increaseRecordCount();
}

bool ReadManager::needNewStream() {
    if(mNumberOfRecord % mEventsPerFile == 0 && mNumberOfRecord != 0)
        return true;
    else
        return false;
}


bool ReadManager::openStream(ofstream& stream) {
    if(stream.is_open())
        closeStream(stream);
    stream.open(formFileName(), stream.binary | stream.trunc);

    DataSetHeader header(mFileType, 52015, mTdcModule->settings());

    trek::serialize(stream, header);

    return true;
}

void ReadManager::closeStream(ofstream& stream) {
    if(stream.is_open()) {
        stream.close();
        increaseFileCount();
    }
}

void ReadManager::resetRecordCount() {
    mNumberOfRecord = 0;
}

void ReadManager::resetFileCount() {
    mNumberOfFiles = 0;
}

void ReadManager::increaseRecordCount() {
    ++mNumberOfRecord;
}

void ReadManager::increaseFileCount()  {
    ++mNumberOfFiles;
}

void ReadManager::setFileType(trek::data::DataSetType type) {
    mFileType = type;
}

uintmax_t ReadManager::getNumberOfRecord() const {
    return mNumberOfRecord;
}

uint64_t ReadManager::getNumberOfRun() const {
    return mNumberOfRun;
}

std::string ReadManager::formFileName() const {
    ostringstream stream;
    stream << mPath << "/"
           << setw(5) << setfill('0') << getNumberOfRun()
           << "_"
           << setw(5) << setfill('0') << mNumberOfFiles
           << ".tds";
    return stream.str();
}

}
//caen
