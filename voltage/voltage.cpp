#include "voltage.hpp"

#include <algorithm>
#include <thread>
#include <cmath>

using std::string;
using std::stoul;
using std::round;
using std::runtime_error;
using std::chrono::system_clock;
using std::chrono::milliseconds;

constexpr unsigned voltageInPin   = 0;
constexpr unsigned voltageOutPin  = 7;
constexpr unsigned amperageOutPin = 6;

Voltage::Voltage()
	: mStream(&mBuffer),
	  mTimeout(Seconds(10)) {
	mStream.exceptions(mStream.badbit | mStream.failbit);
}

Voltage::Voltage(const std::string& name)
	: mBuffer(name),
	  mStream(&mBuffer),
	  mTimeout(Seconds(10)) {
	mStream.exceptions(mStream.badbit | mStream.failbit);
}

void Voltage::open(const std::string& name) {
	mBuffer.open(name);
	mStream.clear();
}

void Voltage::close() {
	mBuffer.close();
}

bool Voltage::isOpen() const {
	return mBuffer.isOpen();
}

double Voltage::voltage() {
	return code2val(readPin(voltageInPin, 100));
}

void Voltage::setAmperage(double val) {
	writePin(amperageOutPin, val2code(val));
}

void Voltage::setVoltage(double val) {
	auto code = readPin(voltageInPin, 10000);
	auto newCode = val2code(val);
	if(newCode == code)
		return;
	auto dir = ((newCode > code) ? 1 : -1);
	while( newCode != code ) {
		code += dir;
		writePin(voltageOutPin, code);
		auto startTime = system_clock::now();
		while( readPin(voltageInPin, 10000) != code) {
			if(system_clock::now() - startTime > mTimeout)
				throw runtime_error("Voltage::setVoltage timeout");
		}
		std::this_thread::sleep_for(milliseconds(200));
	}
}

void Voltage::setTimeout(const Seconds& tm) {
	mTimeout = tm;
}

double Voltage::code2val(unsigned code) {
	return 1.218059405940594e-01*code + 6.277227722772505e-02;
}

unsigned Voltage::val2code(double val) {
	val = 8.209770179674672*val - 5.152849095111165e-01;
	if(val < 0)
		return 0;
	return round(val);
}

unsigned Voltage::in2out(unsigned code) {
	if(code == 0)
		return 0;
	return round(2.645179121649108e-01*code + 7.184796712165564e-01 );
}

void Voltage::writePin(unsigned pin, unsigned code) {
	Lock lk(mMutex);
	mStream << "writePin:" << pin << ',' << code << std::endl;
	std::getline(mStream, mResponseBuffer);
	mResponse.parse(mResponseBuffer);
	if(mResponse.status != 0)
		throw runtime_error("Voltage::writePin failed write pin");
	if(mResponse.command != "writePin" || stoul(mResponse.args.at(0)) != pin || stoul(mResponse.args.at(1)) != code)
		throw runtime_error("Voltage::writePin invalid response");
}

unsigned Voltage::readPin(unsigned pin, unsigned cycles) {
	Lock lk(mMutex);
	mStream << "readPin:" << pin << ',' << cycles << std::endl;
	std::string ans;
	std::getline(mStream, mResponseBuffer);
	mResponse.parse(mResponseBuffer);
	if(mResponse.status != 0)
		throw runtime_error("Voltage::readPin failed write pin");
	if(mResponse.command != "readPin" || stoul(mResponse.args.at(0)) != pin || stoul(mResponse.args.at(1)) != cycles)
		throw runtime_error("Voltage::readPin invalid response");
	return in2out(stoul(mResponse.args.at(2)));
}

// trim from start
static inline std::string &ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
	return ltrim(rtrim(s));
}

void Voltage::Response::parse(const std::string& response) {
	command.clear();
	args.clear();

	auto beg = response.cbegin();
	auto end = response.cend();

	auto it = std::find(beg, end, ' ');
	lexem.assign(beg, it);
	trim(lexem);
	status = std::stoi(lexem);
	if(it == end)
		return;
	beg = it + 1;
	it = std::find(beg, end, ':');
	if(it == end)
		throw std::runtime_error("Voltage::Response::parse invalid command");
	lexem.assign(beg, it);
	trim(lexem);
	command = lexem;
	if(it == end)
		return;
	beg = it + 1;
	do {
		it = std::find(beg, end, ',');
		auto arg = string{beg, it};
		if(trim(arg).empty())
			throw std::runtime_error("Voltage::Response::parse invalid arg");
		args.push_back(arg);
		beg = it + 1;
	} while (it != end);
}
