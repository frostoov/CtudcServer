#include "channelconfig.hpp"

#include <algorithm>

using trek::data::EventHits;
using trek::data::HitRecord;

using std::vector;

vector<EventHits> convertEvents(const vector<Tdc::EventHits>& events, const ChannelConfig& conf) {
    vector<EventHits> newEvents;
    std::transform(events.begin(), events.end(), std::back_inserter(newEvents), [&](auto& eventHits){
        return convertEventHits(eventHits, conf);
    });
    return newEvents;  
}

EventHits convertEventHits(const Tdc::EventHits& hits, const ChannelConfig& conf) {
    EventHits newHits;
    std::transform(hits.begin(), hits.end(), std::back_inserter(newHits), [&](auto& hit){
        return convertHit(hit, conf);
    });
    return newHits;      
}

HitRecord convertHit(const Tdc::Hit& hit, const ChannelConfig& conf) {
    auto& c = conf.at(hit.channel);    
    return HitRecord(convertEdgeDetection(hit.type), c.wire,  c.chamber, hit.time);
}


HitRecord::Type convertEdgeDetection(Tdc::EdgeDetection ed) {
    switch(ed) {
    case Tdc::EdgeDetection::leading:
        return HitRecord::Type::leading;
    case Tdc::EdgeDetection::trailing:
        return HitRecord::Type::trailing;
    default:
        throw std::logic_error("EventWriter::convertEdgeDetection invalid value");
    }
}
