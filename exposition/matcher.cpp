#include "matcher.hpp"

using trek::data::EventHits;
using trek::data::EventRecord;

using std::vector;
using std::make_unique;



auto EventMatcher::makeRecords(const vector<EventHits>& events, int run, int num) const {
	vector<EventRecord> records;
	records.reserve(events.size());

	for(auto& event : events) {
		records.emplace_back(run, num++, event);
	}

	return records;
}

void EventMatcher::load(const vector<Tdc::EventHits> hits, const EventID& id) {
	if(mOpenID == nullptr) {
		mOpenID = make_unique<EventID>(id);
	} else {
		++mFrames;
		mCloseID = make_unique<EventID>(id);
		std::move(hits.begin(), hits.end(), std::back_inserter(mBuffer));
	}
}

bool EventMatcher::unload(vector<EventRecord>& records) {
	records.clear();
	if(mFrames == 0) {
		return true;
	}
	records = makeRecords(convertEvents(mBuffer, mConf), mOpenID->run, mOpenID->num + 1);
	bool matched = mCloseID->run == mOpenID->run && packetCount() == records.size();
	mFrames = 0;
	mBuffer.clear();
	mOpenID = std::move(mCloseID);
	return matched;
}

void EventMatcher::reset() {
	mFrames = 0;
	mBuffer.clear();
	mOpenID.reset();
	mCloseID.reset();
}

uintmax_t EventMatcher::packetCount() const {
	return mCloseID->num - mOpenID->num;
}

