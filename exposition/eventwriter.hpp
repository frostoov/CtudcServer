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


class EventWriter {
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
	EventWriter(const std::string& path,
	            const std::string& prefix,
	            unsigned eventsPerFile,
	            const ChannelConfig& config);
	void write(const RawEvents& tdcEvents, const trek::data::NevodPackage& nvdPkg);
protected:
	void writeBuffer(const RawEvents& buffer, const EventId& eventId);
	trek::data::EventHits convertHits(const RawEvent& eventrecord);
	void reopenStream();
	void openStream();
	void closeStream();
	std::string formFileName() const;
	trek::data::HitRecord::Type convertEdgeDetection(Tdc::EdgeDetection ed);
private:
	EventIdPtr    mNevodId;
	std::ofstream mStream;
	unsigned      mFileCount;
	unsigned      mEventCount;

	const std::string   mPath;
	const std::string   mPrefix;
	const unsigned      mEventsPerFile;
	const ChannelConfig mConfig;
};
