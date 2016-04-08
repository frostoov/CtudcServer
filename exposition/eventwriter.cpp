#include "eventwriter.hpp"

#include <trek/common/stringbuilder.hpp>
#include <trek/common/serialization.hpp>

#include <iostream>
#include <iomanip>

using trek::data::EventRecord;
using trek::StringBuilder;

using std::vector;
using std::string;
using std::setw;
using std::setfill;
using std::ostringstream;
using std::runtime_error;
using std::exception;
using std::logic_error;

EventWriter::EventWriter(const string& path,
                         const string& prefix,
                         unsigned eventsPerFile)
    : mFileCount(0),
      mEventCount(0),
      mPath(path),
      mPrefix(prefix),
      mEventsPerFile(eventsPerFile) {
    mStream.exceptions(mStream.failbit | mStream.badbit);
}

void EventWriter::writeEvent(const EventRecord& record) {
    try {
        if(mEventCount % mEventsPerFile == 0)
            openStream();
        trek::serialize(mStream, record);
        ++mEventCount;
    } catch(const exception& e) {
        std::cerr << "EventWriter::writeEvent " << e.what() << std::endl;
    }
}

void EventWriter::writeDrop(const EventRecord &record) {
    try {
        if(!mDropStream.is_open()) {
            mDropStream.open(StringBuilder() << mPath << '/' << mPrefix << "_drop.tds", mDropStream.binary | mDropStream.trunc);
            mDropStream << "TDSdrop\n";
        }
        trek::serialize(mDropStream, record);
    } catch(std::exception& e) {
        std::cerr << "EventWriter::writeDrop " << e.what() << std::endl;
    }
}

void EventWriter::openStream() {
    if(mStream.is_open()) {
        mStream.close();
        ++mFileCount;
    }
    mStream.open(formFileName(), mStream.binary | mStream.trunc);
    mStream << "TDSa\n";
}

string EventWriter::formFileName() const {
    return StringBuilder() << mPath << '/' << mPrefix
                           << setw(9) << setfill('0') << mFileCount
                           << ".tds";
}
