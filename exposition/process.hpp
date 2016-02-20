#pragma once

class Process {
public:
	virtual ~Process() { };
	virtual void run() = 0;
	virtual void stop() = 0;
protected:
	Process() = default;
};
