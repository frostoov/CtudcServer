//
// Created by frostoov on 10/27/15.
//

#pragma once

#include "tdc/tdc.hpp"
#include "channelconfig.hpp"

#include <trek/data/nevod.hpp>
#include <trek/data/hitrecord.hpp>
#include <trek/data/eventrecord.hpp>
#include <memory>
#include <fstream>


class EventEncoder {
	struct EventId {
		EventId(unsigned r, unsigned e)
			: nRun(r), nRecord(e) { }
		unsigned nRun;
		unsigned nRecord;
	};
	using EventIdPtr = std::unique_ptr<EventId>;
	using RawEvent = Tdc::EventHits;
	using RawEvents = std::vector<RawEvent>;
public:
	EventEncoder(const std::string& path,
	             unsigned eventsPerFile,
			 	 const ChannelConfig& config);
	void encode(const RawEvents& tdcEvents, const trek::data::NevodPackage& nvdPkg);
protected:
	void writeBuffer(const RawEvents& buffer, const EventId& eventId);
	trek::data::EventHits convertHits(const RawEvent& eventrecord);
	void reopenStream();
	void openStream();
	void closeStream();
	void newRun(const trek::data::NevodPackage& nvdPkg);
	std::string formFileName() const;
private:
	EventIdPtr    mNevodId;
	std::ofstream mStream;
	unsigned      mFileCount;
	unsigned      mEventCount;

	const std::string   mPath;
	const unsigned      mEventsPerFile;
	const ChannelConfig mConfig;
};
