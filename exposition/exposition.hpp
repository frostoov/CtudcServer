#pragma once

#include "channelconfig.hpp"
#include "freq.hpp"
#include "process.hpp"

#include "tdc/tdc.hpp"

#include <trek/data/eventrecord.hpp>
#include <trek/net/multicastreceiver.hpp>

#include <condition_variable>
#include <atomic>
#include <thread>

class Exposition : public Process {
public:
    virtual ~Exposition() { }
    virtual operator bool() const = 0;
    virtual uintmax_t triggerCount() const = 0;
    virtual uintmax_t triggerDrop() const = 0;
    virtual uintmax_t packageCount() const = 0;
    virtual uintmax_t packageDrop() const = 0;

    virtual TrekHitCount chambersCount() const = 0;
    virtual TrekHitCount chambersDrop() const = 0;
protected:
    Exposition() = default;
};

TrekFreq convertFreq(const ChannelFreq& freq, const ChannelConfig& conf);
void printStartMeta(const std::string& dir, unsigned run, Tdc& module);
void printEndMeta(const std::string& dir, unsigned run);
std::string runPath(const std::string& dir, unsigned run);
std::string metaPath(const std::string& dir, unsigned run);
std::string formatPrefix(unsigned run);
