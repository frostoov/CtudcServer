#include "voltage.hpp"

#include <algorithm>
#include <cmath>

using std::string;
using std::stoul;
using std::round;
using std::runtime_error;
using std::chrono::system_clock;

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
}

void Voltage::close() {
	mBuffer.close();
}

bool Voltage::isOpen() const {
	return mBuffer.isOpen();
}

void Voltage::turnOn() {
	setVoltage(12);
}

void Voltage::turnOff() {
	setVoltage(0);
}

double Voltage::voltage() {
	return code2val(readPin(voltageInPin, 100));
}

void Voltage::setTimeout(const Seconds& tm) {
	mTimeout = tm;
}


void Voltage::setVoltage(double val) {
	auto code = readPin(voltageInPin, 100);
	auto newCode = val2code(val);
	if(newCode == code)
		return;
	auto dir = ((newCode > code) ? 1 : -1);
	while( newCode != code ) {
		code += dir;
		if(code == 6) {
			auto c = ((newCode > code) ? 2 : 1);
			writePin(amperageOutPin, c);
		}
		writePin(voltageOutPin, code);
		auto startTime = system_clock::now();
		while( readPin(voltageInPin, 100) != code) {
			if(system_clock::now() - startTime > mTimeout)
				throw runtime_error("Voltage::setVoltage timeout");
		}
	}
}

double Voltage::code2val(unsigned code) {
	return 3.221985345674413e-02*code + 1.502873020242141e-01;
}

unsigned Voltage::val2code(double val) {
	val = 8.209770179674672*val - 5.152849095111165e-01;
	if(val < 0)
		return 0;
	return round(val);
}

unsigned Voltage::in2out(unsigned code) {
	return round(2.645179121649108e-01*code + 7.184796712165564e-01 );
}

void Voltage::writePin(unsigned pin, unsigned code) {
	mStream << "writePin:" << pin << ',' << code << std::endl;
	std::string ans;
	std::getline(mStream, ans);
	mResponse.parse(ans);
	if(mResponse.status != 0)
		throw runtime_error("Voltage::writePin failed write pin");
	if(mResponse.command != "writePin" || stoul(mResponse.args.at(0)) != pin || stoul(mResponse.args.at(1)) != code)
		throw runtime_error("Voltage::writePin invalid response " + ans);
}

unsigned Voltage::readPin(unsigned pin, unsigned cycles) {
	mStream << "readPin:" << pin << ',' << cycles << std::endl;
	std::string ans;
	std::getline(mStream, ans);
	mResponse.parse(ans);
	if(mResponse.status != 0)
		throw runtime_error("Voltage::readPin failed write pin");
	if(mResponse.command != "readPin" || stoul(mResponse.args.at(0)) != pin || stoul(mResponse.args.at(1)) != cycles)
		throw runtime_error("Voltage::readPin invalid response " + ans);
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
