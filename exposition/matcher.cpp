#include "matcher.hpp"

#include <algorithm>
#include <numeric>

using trek::data::EventHits;
using trek::data::EventRecord;

using std::vector;
using std::deque;
using std::make_unique;
using std::accumulate;
using std::transform;
using std::runtime_error;


auto triggerCount(const deque<vector<Tdc::EventHits>>& buf) {
    return accumulate(buf.begin(), buf.end(), uintmax_t(0), [](const auto& a, const auto& b) { return a + b.size(); });
}

auto packetCount(const deque<EventID>& ids) {
    if (ids.empty()) {
        return uintmax_t(0);
    }
    return ids.back().num - ids.front().num;
}


auto validateGate(const deque<vector<Tdc::EventHits>>& buf, const deque<EventID>& ids) {
    return ids.back().run == ids.front().run
        && packetCount(ids) == triggerCount(buf);
}

auto EventMatcher::makeRecords(const vector<EventHits>& events, int run, int num) const {
	vector<EventRecord> records;
	records.reserve(events.size());

	for(auto& event : events) {
		records.emplace_back(run, num++, event);
	}

	return records;
}

bool EventMatcher::load(const vector<Tdc::EventHits>& hits, const EventID& id) {
    mEvents.push_back(id);
    mBuffer.push_back(hits);
    if (mEvents.size() == 1) {
        mBuffer.clear();
        return false;
    } 
    
    matched = validateGate(mBuffer, mEvents);
    if (matched) {
        return true;
    } 
    if (mEvents.size() == 3) {
        mEvents.pop_front();
        mBuffer.pop_front();
        matched = validateGate(mBuffer, mEvents);
    }
    return matched;
}

bool EventMatcher::unload(vector<EventRecord>& records) {
    auto& front = mEvents.at(0);
    std::vector<EventHits> buf;
    for(const auto& raw : mBuffer) {
        auto events = convertEvents(raw, mConf);
        std::move(events.begin(), events.end(), back_inserter(buf));
    }
	records = makeRecords(buf, front.run, front.num + 1);
    mEvents.clear();
    mBuffer.clear();
	return matched;
}

void EventMatcher::reset() {
    mBuffer.clear();
    mEvents.clear();
}

uintmax_t EventMatcher::triggers() const {
    return triggerCount(mBuffer);
}

uintmax_t EventMatcher::frames() const {
    return packetCount(mEvents);
}
