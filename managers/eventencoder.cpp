//
// Created by frostoov on 10/27/15.
//

#include "eventencoder.hpp"

#include <trek/data/eventrecord.hpp>
#include <trek/common/applog.hpp>

#include <sstream>
#include <iomanip>
#include <cassert>

using trek::data::NevodPackage;
using trek::data::EventRecord;

using std::make_unique;
using std::vector;
using std::string;
using std::setw;
using std::setfill;
using std::ostringstream;
using std::runtime_error;
using std::exception;

EventEncoder::EventEncoder(const string& path,
                           unsigned eventsPerFile,
                           const ChannelConfig& config)
	: mFileCount(0),
	  mEventCount(0),
	  mPath(path),
	  mEventsPerFile(eventsPerFile),
	  mConfig(config) {
	mStream.exceptions(mStream.failbit | mStream.badbit);
}

void EventEncoder::encode(const RawEvents& tdcEvents, const NevodPackage& nvdPkg) {
	try {
		if(mNevodId == nullptr || nvdPkg.numberOfRun != mNevodId->nRun) {
			mNevodId = make_unique<EventId>(EventId{nvdPkg.numberOfRun, nvdPkg.numberOfRecord});
			return;
		}
		if(nvdPkg.numberOfRecord - mNevodId->nRecord == tdcEvents.size())
			writeBuffer(tdcEvents, *mNevodId);
		mNevodId = make_unique<EventId>(EventId{nvdPkg.numberOfRun, nvdPkg.numberOfRecord});
	} catch(const exception& e) {
		trek::Log::instance() << "EventEncoder::encode " << e.what() << std::endl;
	}
}

void EventEncoder::writeBuffer(const RawEvents& buffer, const EventId& eventId) {
	if(!mStream.is_open())
		openStream();
	auto curEvent = eventId.nRecord;
	for(auto& evt : buffer) {
		EventRecord record(eventId.nRun, curEvent++, convertHits(evt));
		if(mEventCount % mEventsPerFile == 0 && mEventCount != 0)
			reopenStream();
		trek::serialize(mStream, record);
		++mEventCount;
	}
}

trek::data::EventHits EventEncoder::convertHits(const Tdc::EventHits& hits) {
	trek::data::EventHits newHits;
	for(auto& hit : hits) {
		if(mConfig.count(hit.channel)) {
			auto& conf = mConfig.at(hit.channel);
			newHits.push_back({conf.wire, conf.chamber, hit.time});
		}
	}
	return newHits;
}

void EventEncoder::openStream() {
	assert(!mStream.is_open());
	mStream.open(formFileName(), mStream.binary);
	mStream << "TDS";
}

void EventEncoder::closeStream() {
	assert(mStream.is_open());
	mStream.close();
	++mFileCount;
}

void EventEncoder::reopenStream() {
	closeStream();
	openStream();
}

std::string EventEncoder::formFileName() const {
	ostringstream str;
	str << mPath << "set_"
	    << setw(9) << setfill('0') << mFileCount
	    << ".tds";
	return str.str();
}
