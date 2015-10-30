#pragma once

#include "serialbuf.hpp"
#include <vector>
#include <iostream>
#include <chrono>


class Voltage {
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

	void turnOn();
	void turnOff();
	void setTimeout(const Seconds& time);
	double voltage();
protected:
	void setVoltage(double val);
	static double code2val(unsigned code);
	static unsigned val2code(double val);
	static unsigned in2out(unsigned code);

	void writePin(unsigned pin, unsigned code);
	unsigned readPin(unsigned pin, unsigned cycles);

private:
	serialbuf     mBuffer;
	std::iostream mStream;
	Response      mResponse;

	Seconds mTimeout;
};
