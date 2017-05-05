#pragma once

#include "channelconfig.hpp"

#include <trek/data/eventrecord.hpp>

#include <vector>
#include <deque>
#include <memory>

struct EventID {
	EventID(uintmax_t num_, uintmax_t run_) 
		: num(num_), run(run_) { }
	const uintmax_t num;
	const uintmax_t run;
};

class EventMatcher {
public:
    EventMatcher(const ChannelConfig& conf);
	void load(const std::vector<Tdc::EventHits>& hits, const EventID& id);
	void unload(std::vector<trek::data::EventRecord> records[2], size_t packetCount[2]);
	void reset();
protected:
	auto makeRecords(const std::vector<trek::data::EventHits>& events, int run, int num) const;
    void verifyBufferLength(size_t n) const;
    void dump(const std::vector<Tdc::EventHits>& src, const EventID& eid, bool matched);
private:
	std::deque<std::vector<Tdc::EventHits>> mBuffer;
    std::vector<trek::data::EventRecord> mRecords[2];
    size_t mFrameCount[2];
    std::deque<EventID> mEvents;
	ChannelConfig mConf;
};
