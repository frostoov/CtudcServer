#pragma once

#include <cstdint>
#include <chrono>

class Process {
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;
    virtual ~Process() { };
    virtual void stop() = 0;
    virtual operator bool() const = 0;
    virtual TimePoint startPoint() const = 0;
protected:
    Process() = default;
};
