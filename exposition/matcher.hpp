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
	EventMatcher(const ChannelConfig& conf)
		: mConf(conf) { }
	bool load(const std::vector<Tdc::EventHits>& hits, const EventID& id);
	bool unload(std::vector<trek::data::EventRecord>& events);
	void reset();
    uintmax_t triggers() const;
    uintmax_t frames() const;
protected:
	auto makeRecords(const std::vector<trek::data::EventHits>& events, int run, int num) const;
    void verifyBufferLength(size_t n) const;
private:
	std::deque<std::vector<Tdc::EventHits>> mBuffer;
    std::deque<EventID> mEvents;
	ChannelConfig mConf;
    bool matched;
};
