#pragma once

#include "tdc/tdc.hpp"

#include <unordered_map>
#include <functional>
#include <chrono>
#include <memory>


using ChannelFreq = std::unordered_map<unsigned, double>;

std::function<ChannelFreq()> launchFreq(std::shared_ptr<Tdc> device, std::chrono::microseconds delay);

