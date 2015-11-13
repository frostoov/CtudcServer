//
// Created by frostoov on 10/27/15.
//

#include "eventwriter.hpp"

#include <trek/data/eventrecord.hpp>
#include <trek/common/serialization.hpp>

#include <sstream>
#include <iostream>
#include <iomanip>
#include <cassert>

using trek::data::EventRecord;

using std::make_unique;
using std::vector;
using std::string;
using std::setw;
using std::setfill;
using std::ostringstream;
using std::runtime_error;
using std::exception;

EventWriter::EventWriter(const string& path,
                         const string& prefix,
                         unsigned eventsPerFile,
                         const ChannelConfig& config)
	: mFileCount(0),
	  mEventCount(0),
	  mPath(path),
	  mPrefix(prefix),
	  mEventsPerFile(eventsPerFile),
	  mConfig(config) {
	mStream.exceptions(mStream.failbit | mStream.badbit);
}

void EventWriter::write(const RawEvents& tdcEvents) {
	try {
		writeBuffer(tdcEvents);
	} catch(const exception& e) {
		std::cerr << "EventWriter::write " << e.what() << std::endl;
	}
}

void EventWriter::writeBuffer(const RawEvents& buffer) {
	if(!mStream.is_open())
		openStream();
	for(auto& evt : buffer) {
		if(mEventCount % mEventsPerFile == 0 && mEventCount != 0)
			reopenStream();
		EventRecord record(0, mEventCount, convertHits(evt));
		trek::serialize(mStream, record);
		++mEventCount;
	}
}

trek::data::EventHits EventWriter::convertHits(const Tdc::EventHits& hits) {
	trek::data::EventHits newHits;
	for(auto& hit : hits) {
		if(mConfig.count(hit.channel)) {
			auto& conf = mConfig.at(hit.channel);
			newHits.push_back({conf.wire, conf.chamber, hit.time});
		} else {
			std::cerr << "Failed find channel config: " << hit.channel  << std::endl;
		}
	}
	return newHits;
}

void EventWriter::openStream() {
	assert(!mStream.is_open());
	mStream.open(formFileName(), mStream.binary | mStream.trunc);
	mStream << "TDSa\n";
}

void EventWriter::closeStream() {
	assert(mStream.is_open());
	mStream.close();
	++mFileCount;
}

void EventWriter::reopenStream() {
	closeStream();
	openStream();
}

std::string EventWriter::formFileName() const {
	ostringstream str;
	str << mPath << '/' << mPrefix
	    << setw(9) << setfill('0') << mFileCount
	    << ".tds";
	return str.str();
}
