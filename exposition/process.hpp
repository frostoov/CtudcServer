#pragma once

#include <cstdint>
#include <chrono>

class Process {
public:
	virtual ~Process() { };
	virtual void run() = 0;
	virtual void stop() = 0;
	virtual bool running() = 0;
	virtual std::chrono::milliseconds duration() const = 0;
protected:
	Process() = default;
};
