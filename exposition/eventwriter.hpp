//
// Created by frostoov on 10/27/15.
//

#pragma once

#include "tdc/tdc.hpp"
#include "channelconfig.hpp"

#include <trek/data/hitrecord.hpp>
#include <trek/data/eventrecord.hpp>
#include <fstream>


class EventWriter {
	using RawEvent = Tdc::EventHits;
	using RawEvents = std::vector<RawEvent>;
public:
	EventWriter(const std::string& path,
	            const std::string& prefix,
	            unsigned eventsPerFile,
	            const ChannelConfig& config);
	void write(const RawEvents& tdcEvents);
protected:
	void writeBuffer(const RawEvents& buffer);
	trek::data::EventHits convertHits(const RawEvent& eventRecord);
	void reopenStream();
	void openStream();
	void closeStream();
	std::string formFileName() const;
private:
	std::ofstream mStream;
	unsigned      mFileCount;
	unsigned      mEventCount;

	const std::string   mPath;
	const std::string   mPrefix;
	const unsigned      mEventsPerFile;
	const ChannelConfig mConfig;
};
