#pragma once

class ProcessManager {
public:
	virtual ~ProcessManager() { };
	virtual void run() = 0;
	virtual void stop() = 0;
protected:
	ProcessManager() = default;
};
