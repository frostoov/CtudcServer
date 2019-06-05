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

auto frameCount(const deque<EventID>& ids) {
    if (ids.empty()) {
        return uintmax_t(0);
    }
    return ids.back().num - ids.front().num;
}


auto validateGate(const deque<vector<Tdc::EventHits>>& buf, const deque<EventID>& ids) {
    return ids.back().run == ids.front().run
        && frameCount(ids) == triggerCount(buf);
}

EventMatcher::EventMatcher(const ChannelConfig& conf)
    : mConf(conf) {
    std::fill(std::begin(mFrameCount), std::end(mFrameCount), 0);
}

auto EventMatcher::makeRecords(const vector<EventHits>& events, int run, int num) const {
	vector<EventRecord> records;
	records.reserve(events.size());

	for(auto& event : events) {
		records.emplace_back(run, num++, event);
	}

	return records;
}

void EventMatcher::load(const vector<Tdc::EventHits>& hits, const EventID& id) {
    mEvents.push_back(id);
    mBuffer.push_back(hits);
    if (mEvents.size() == 1) {
        mBuffer.clear();
        return;
    } 
    
    while(mEvents.size() > 1) {
        auto matched = validateGate(mBuffer, mEvents);
        if (matched) {
            while(!mBuffer.empty()) {
                dump(mBuffer.at(0), mEvents.at(0), matched);
                mBuffer.pop_front();
                mEvents.pop_front();
            }
        } else if (mEvents.size() == 3) {
            dump(mBuffer.at(0), mEvents.at(0), matched);
            mEvents.pop_front();
            mBuffer.pop_front();
        } else {
            break;
        }
    }
}

void EventMatcher::dump(const std::vector<Tdc::EventHits>& src, const EventID& eid, bool matched) {
    size_t i = bool(matched);
    auto events = convertEvents(src, mConf);
    auto records = makeRecords(events, eid.run, eid.num + 1);
    std::move(records.begin(), records.end(), std::back_inserter(mRecords[i]));
    ++mFrameCount[i];
}

void EventMatcher::unload(vector<EventRecord> records[2], size_t frameCount[2]) {
    for (size_t i = 0; i < 2; ++i) {
        records[i] = std::move(mRecords[i]);
        frameCount[i] = mFrameCount[i];
        mRecords[i].clear();
        mFrameCount[i] = 0;
    }
}
