#include "amplifier.hpp"

#include <sstream>
#include <iterator>
#include <cmath>

using std::set;
using std::string;
using std::istringstream;
using std::istream_iterator;
using std::round;
using std::ostream;

Amplifier::Amplifier()
    : mStream(&mBuffer) {
    mStream.exceptions(mStream.badbit | mStream.failbit);
}

void Amplifier::open(const string& name) {
    mStream.clear();
    mCellStats.clear();
	mBuffer.open(name, 9600);

    mCellStats = readCellStats(readCellNums());
}

void Amplifier::close() {
    mCellStats.clear();
    mBuffer.close();
}

Amplifier::Stat Amplifier::stat(int cell) {
    return Amplifier::Stat(readByte(cell, 7));
}

void Amplifier::setVoltage(int cell, double code) {
    writeWord(cell, 1, volt2code(cell, code));
    writeByte(cell, 0, 1);
}

void Amplifier::setAmperage(int cell, double amp) {
    writeWord(cell, 1, amp2code(cell, amp));
    writeByte(cell, 0, 1);
}

void Amplifier::setSpeedUp(int cell, uint8_t code) {
    writeByte(cell, 10, code);
}

void Amplifier::setSpeedDn(int cell, uint8_t code) {
    writeByte(cell, 11, code);
}

double Amplifier::speedUp(int cell) {
    return code2speed(cell, readByte(cell, 10));
}

double Amplifier::speedDn(int cell) {
    return code2speed(cell, readByte(cell, 11));
}


void Amplifier::turnOn(int cell) {
    writeByte(cell, 0, 4);
}

void Amplifier::turnOff(int cell) {
    writeByte(cell, 0, 5);
}

double Amplifier::voltage(int cell) {
    auto code = readWord(cell, 5);
    return code2volt(cell,code);
}

double Amplifier::amperage(int cell) {
    auto code = readWord(cell, 8);
    return code2amp(cell, code);
}

inline bool startsWith(const string& str, const string& starting) {
	return str.find(starting) == 0;
}

inline bool endsWith(const string& str, const string& ending) {
	if(ending.size() >= str.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), str.rend());
}

set<int> Amplifier::readCellNums() {
    mStream << 'l' << std::flush;
    string response;
    std::getline(mStream, response);
    if(startsWith(response, "Er"))
        throw std::runtime_error("Amplifier::readCellNums failed: " + response);
    istringstream iss(response);
    set<int> cellNums;
    for(auto it = istream_iterator<string>(iss); it != istream_iterator<string>(); ++it) {
        cellNums.insert(std::stoi(*it));
	}
    return cellNums;
}

Amplifier::CellStats Amplifier::readCellStats(const set<int>& cells) {
    CellStats cellStats;
    for(auto cell : cells) {
        auto uMesmax = readWord(cell, 22);
        auto uMax    = readWord(cell, 18);
        auto uMin    = readWord(cell, 16);
        auto iMesmax = readWord(cell, 24);
        auto iMax    = readWord(cell, 20);
        cellStats.emplace(cell, CellStat{uMesmax, uMax, uMin, iMesmax, iMax});
    }
    return cellStats;
}

void Amplifier::writeWord(int cell, int addr, uint16_t word) {
    mStream << 'w' << std::hex << cell << ',' << addr << ',' << word << '\r' << std::flush;
    string response;
    std::getline(mStream, response);
    if(std::stoul(response, nullptr, 16) != word)
        throw std::runtime_error("Amplifier::writeWord failed: " + response);
}

void Amplifier::writeByte(int cell, int addr, uint8_t byte) {
    mStream << '>' << std::hex << cell << ',' << addr << ',' << uint16_t(byte) << '\r' << std::flush;
    string response;
    std::getline(mStream, response);
    if(std::stoul(response, nullptr, 16) != byte)
        throw std::runtime_error("Amplifier::writeWord failed: " + response);
}

uint16_t Amplifier::readWord(int cell, int addr) {
    mStream << 'r' << std::hex << cell << ',' << addr << '\r' << std::flush;
    string response;
    std::getline(mStream, response);
    if(startsWith(response, "Er"))
        throw std::runtime_error("Amplifier::readWord failed: " + response);
    return uint16_t( std::stoi(response, nullptr, 16) );
}

uint8_t Amplifier::readByte(int cell, int addr) {
    mStream << '<' << std::hex << cell << ',' << addr << '\r' << std::flush;
    string response;
    std::getline(mStream, response);
    if(startsWith(response, "Er"))
        throw std::runtime_error("Amplifier::readByte failed: " + response);
    return uint8_t( std::stoi(response, nullptr, 16) );
}

double Amplifier::code2volt(int cell, uint16_t code) {
    auto& stat = mCellStats.at(cell);
    return double(stat.umesmax)/4095*(code&0xfff);
}

double Amplifier::code2amp(int cell, uint16_t code) {
    auto& stat = mCellStats.at(cell);
    return double(stat.imesmax)/1023*code;
}

uint16_t Amplifier::volt2code(int cell, double volt) {
    auto& stat = mCellStats.at(cell);
    return round( 1023*(volt-stat.umin)/(stat.umax - stat.umin) );
}

uint16_t Amplifier::amp2code(int cell, double amp) {
    auto& stat = mCellStats.at(cell);
    return round( amp*1023/stat.imax );
}

uint16_t Amplifier::speed2code(int cell, double speed) {
    auto& stat = mCellStats.at(cell);
    return round( double(stat.umax - stat.umin)*200/speed );
}

double Amplifier::code2speed(int cell, uint16_t code) {
    auto& stat = mCellStats.at(cell);
    return double(stat.umax - stat.umin)*200/code;
}


ostream& operator<<(ostream& stream, Amplifier::Stat stat) {
    return stream << "CEB:   " << stat.ceb() << '\n'
                  << "GS:    " << stat.gs() << '\n'
                  << "ACCEB: " << stat.acceb() << '\n'
                  << "IOVLD: " << stat.iovld() << '\n'
                  << "STDBY: " << stat.stdby() << '\n'
                  << "RDACT: " << stat.rdact() << '\n'
                  << "RUACT: " << stat.ruact() << '\n';
}
