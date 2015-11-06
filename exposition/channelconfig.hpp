//
// Created by frostoov on 10/27/15.
//

#pragma once

#include <unordered_map>

struct ChannelCongruence {
	ChannelCongruence(unsigned c, unsigned w)
		: chamber(c), wire(w) { }
	unsigned chamber;
	unsigned wire;
};

using ChannelConfig = std::unordered_map<unsigned , ChannelCongruence>;
