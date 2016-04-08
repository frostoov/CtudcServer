#pragma once

#include <unordered_map>

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
