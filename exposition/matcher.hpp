#pragma once

#include "channelconfig.hpp"

#include <trek/data/eventrecord.hpp>

#include <vector>
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
	void load(const std::vector<Tdc::EventHits> hits, const EventID& id);
	uintmax_t frames() const { return mFrames; }
	uintmax_t triggers() const { return mBuffer.size(); }
	bool unload(std::vector<trek::data::EventRecord>& events);
	void reset();
protected:
	uintmax_t packetCount() const;
	auto makeRecords(const std::vector<trek::data::EventHits>& events, int run, int num) const;
private:
	uintmax_t mFrames = 0;
	std::unique_ptr<EventID> mOpenID = nullptr;
	std::unique_ptr<EventID> mCloseID = nullptr;
	std::vector<Tdc::EventHits> mBuffer;
	ChannelConfig mConf;
};
