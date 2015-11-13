#pragma once

#include "serialbuf.hpp"
#include <iostream>
#include <mutex>


class Voltage {
	using Mutex = std::recursive_mutex;
	using Lock = std::lock_guard<Mutex>;
public:
	struct Response;
public:
	Voltage();
	Voltage(const std::string& name);

	void open(const std::string& name);
	void close();
	bool isOpen() const;

	void setVoltage(double val);
	void setAmperage(double val);
	void setTimeout(int timeout);
	double voltage(int count);
protected:
	static double code2val(unsigned code);
	static unsigned val2code(double val);
	static unsigned in2out(unsigned code);

	void writePin(unsigned pin, unsigned code);
	unsigned readPin(unsigned pin, unsigned cycles);
private:
	serialbuf     mBuffer;
	std::iostream mStream;

	Mutex mMutex;
};
