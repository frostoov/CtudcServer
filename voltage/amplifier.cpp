#include "amplifier.hpp"

#include <boost/asio.hpp>

#include <sstream>
#include <iterator>
#include <cmath>
#include <thread>

namespace asio = boost::asio;

using std::set;
using std::string;
using std::istringstream;
using std::istream_iterator;
using std::round;
using std::ostream;
using std::mutex;
using std::lock_guard;

static constexpr int baudRate = 9600;

Amplifier::Amplifier() : mPort(mIoService) {}

Amplifier::Amplifier(const string& name)
    : mPort(mIoService) {
    open(name);
}

Amplifier::~Amplifier() {
    close();
}

void Amplifier::open(const string& name) {
    lock_guard<mutex> lk(mStateMutex);
    mPort.open(name);
    mPort.set_option(boost::asio::serial_port::baud_rate(baudRate));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    mCellStats.clear();

    mCellStats = readCellStats(readCellNums());
}

void Amplifier::close() {
    lock_guard<mutex> lk(mStateMutex);
    mCellStats.clear();
    mPort.close();
}

Amplifier::Stat Amplifier::stat(int cell) {
    return Amplifier::Stat(readByte(cell, 7));
}

void Amplifier::setVoltage(int cell, int volt) {
    auto code = volt2code(cell, volt);
    if(code > 1023)
        throw std::runtime_error("Amplifier::setVoltage invalid voltage");
    writeWord(cell, 1, code);
    writeByte(cell, 0, 1);
}

void Amplifier::setAmperage(int cell, int amp) {
    auto code = amp2code(cell, amp);
    if(code > 1023)
        throw std::runtime_error("Amplifier::setAmperage invalid amperage");
    writeWord(cell, 1, code);
    writeByte(cell, 0, 1);
}

void Amplifier::setSpeedUp(int cell, int speed) {
    auto code = speed2code(cell, speed);
    if(code == 0 || code > 255)
        throw std::runtime_error("Amplifier::setSpeedUp invalid speed");
    writeByte(cell, 10, code);
}

void Amplifier::setSpeedDn(int cell, int speed) {
    auto code = speed2code(cell, speed);
    if(code > 255)
        throw std::runtime_error("Amplifier::setSpeedUp invalid speed");
    writeByte(cell, 11, code);
}

int Amplifier::speedUp(int cell) {
    return code2speed(cell, readByte(cell, 10));
}

int Amplifier::speedDn(int cell) {
    return code2speed(cell, readByte(cell, 11));
}

void Amplifier::turnOn(int cell) {
    writeByte(cell, 0, 4);
}

void Amplifier::turnOff(int cell) {
    writeByte(cell, 0, 5);
}

int Amplifier::voltage(int cell) {
    auto code = readWord(cell, 5) & 0xfff;
    return code2volt(cell, code);
}

int Amplifier::amperage(int cell) {
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

static string& replace(string& str, const string& s, const string& t) {
    string::size_type pos = 0;
    while((pos = str.find(s, pos)) != str.npos) {
        str.replace(pos, s.size(), t);
        pos += t.size();
    }
    return str;
}

std::string Amplifier::send(asio::serial_port& port, const std::string& msg) {
    asio::streambuf buffer;
    {
        lock_guard<mutex> lk(mMutex);
        asio::write(port, asio::buffer(msg));
        asio::read_until(port, buffer, '\n');
    }
    std::istream istr(&buffer);
    string response;
    std::getline(istr, response);
    return response;
}

set<int> Amplifier::readCellNums() {
    auto response = send(mPort, "l");
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
    std::ostringstream ostr;
    ostr << 'w' << std::hex << cell << ',' << addr << ',' << word << '\r';
    auto response = send(mPort, ostr.str());
    if(std::stoul(response, nullptr, 16) != word)
        throw std::runtime_error("Amplifier::writeWord failed: " + response);
}

void Amplifier::writeByte(int cell, int addr, uint8_t byte) {
    std::ostringstream ostr;
    ostr << '>' << std::hex << cell << ',' << addr << ',' << uint16_t(byte) << '\r';
    auto response = send(mPort, ostr.str());
    if(std::stoul(response, nullptr, 16) != byte)
        throw std::runtime_error("Amplifier::writeWord failed: " + response);
}

uint16_t Amplifier::readWord(int cell, int addr) {
    std::ostringstream ostr;
    ostr << 'r' << std::hex << cell << ',' << addr << '\r';
    auto response = send(mPort, ostr.str());
    if(startsWith(response, "Er"))
        throw std::runtime_error("Amplifier::readWord failed: " + response);
    return uint16_t( std::stoi(response, nullptr, 16) );
}

uint8_t Amplifier::readByte(int cell, int addr) {
    std::ostringstream ostr;
    ostr << '<' << std::hex << cell << ',' << addr << '\r';
    auto response = send(mPort, ostr.str());
    if(startsWith(response, "Er"))
        throw std::runtime_error("Amplifier::readByte failed: " + response);
    return uint8_t( std::stoi(response, nullptr, 16) );
}

int Amplifier::code2volt(int cell, uint16_t code) {
    auto& stat = mCellStats.at(cell);
    return round( double(stat.umesmax) / 4095 * code );
}

int Amplifier::code2amp(int cell, uint16_t code) {
    auto& stat = mCellStats.at(cell);
    return round( double(stat.imesmax) / 1023 * code );
}

uint16_t Amplifier::volt2code(int cell, int volt) {
    auto& stat = mCellStats.at(cell);
    return round( 1023 * (volt - stat.umin) / (stat.umax - stat.umin) );
}

uint16_t Amplifier::amp2code(int cell, int amp) {
    auto& stat = mCellStats.at(cell);
    return round( amp * 1023 / stat.imax );
}

uint16_t Amplifier::speed2code(int cell, int speed) {
    auto& stat = mCellStats.at(cell);
    return round( double(stat.umax - stat.umin) * 200 / (speed * 1024) );
}

int Amplifier::code2speed(int cell, uint16_t code) {
    auto& stat = mCellStats.at(cell);
    return round( double(stat.umax - stat.umin) * 200 / (code * 1024) );
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
