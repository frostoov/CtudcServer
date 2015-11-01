#pragma once

#include "serialbuf.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <mutex>


class Voltage {
	using Mutex = std::recursive_mutex;
	using Lock = std::lock_guard<Mutex>;
public:
	using Seconds = std::chrono::seconds;
	struct Response {
		int status;
		std::string command;
		std::vector<std::string> args;

		void parse(const std::string& str);
	private:
		std::string lexem;
	};
public:
	Voltage();
	Voltage(const std::string& name);

	void open(const std::string& name);
	void close();
	bool isOpen() const;

	void setVoltage(double val);
	void setAmperage(double val);
	void setTimeout(const Seconds& time);
	double voltage();
protected:

	static double code2val(unsigned code);
	static unsigned val2code(double val);
	static unsigned in2out(unsigned code);

	void writePin(unsigned pin, unsigned code);
	unsigned readPin(unsigned pin, unsigned cycles);

private:
	serialbuf     mBuffer;
	std::iostream mStream;
	std::string   mResponseBuffer;
	Response      mResponse;

	Seconds mTimeout;
	Mutex mMutex;
};
