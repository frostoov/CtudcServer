#pragma once

#include "tdc/tdc.hpp"

#include <trek/data/eventrecord.hpp>

#include <unordered_map>
#include <vector>

struct ChannelCongruence {
    ChannelCongruence(unsigned c, unsigned w)
        : chamber(c), wire(w) { }
    unsigned chamber;
    unsigned wire;
};

using ChannelConfig = std::unordered_map<unsigned, ChannelCongruence>;

using ChamberFreq = std::array<double, 4>;
using TrekFreq = std::unordered_map<unsigned, ChamberFreq>;

using ChamberHitCount = std::array<uintmax_t, 4>;
using TrekHitCount = std::unordered_map<unsigned, ChamberHitCount>;

std::vector<trek::data::EventHits> convertEvents(const std::vector<Tdc::EventHits>& events, const ChannelConfig& conf);
trek::data::EventHits convertEventHits(const Tdc::EventHits& hits, const ChannelConfig& conf);
trek::data::HitRecord convertHit(const Tdc::Hit& hit, const ChannelConfig& conf);
trek::data::HitRecord::Type convertEdgeDetection(Tdc::EdgeDetection ed);
