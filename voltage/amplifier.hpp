#pragma once

#include <boost/asio/serial_port.hpp>
#include <boost/asio/streambuf.hpp>

#include <mutex>
#include <iostream>
#include <unordered_map>
#include <set>

class Amplifier {
    struct CellStat {
        uint16_t umesmax;
        uint16_t umax;
        uint16_t umin;

        uint16_t imesmax;
        uint16_t imax;
    };
    using CellStats = std::unordered_map<int, CellStat>;
public:
    class Stat {
    public:
        explicit Stat(uint8_t stat) : mStat(stat) {}
        bool ceb() const   { return (mStat >> 0) & 1; }
        bool gs() const    { return (mStat >> 1) & 1; }
        bool acceb() const { return (mStat >> 2) & 1; }
        bool iovld() const { return (mStat >> 3) & 1; }
        bool stdby() const { return (mStat >> 4) & 1; }
        bool rdact() const { return (mStat >> 6) & 1; }
        bool ruact() const { return (mStat >> 7) & 1; }
        uint8_t value() const { return mStat; }
    private:
        uint8_t mStat;
    };
public:
    Amplifier();

    void open(const std::string& name);
    void close();
    bool isOpen() const { return mPort.is_open(); }

    Stat stat(int cell);
    const CellStats& cellStats() const { return mCellStats; }

    void setVoltage(int cell,  int volt);
    void setAmperage(int cell, int amp);

    void setSpeedUp(int cell, int speed);
    void setSpeedDn(int cell, int speed);

    int speedUp(int cell);
    int speedDn(int cell);

    int voltage(int cell);
    int amperage(int cell);

    void turnOn(int cell);
    void turnOff(int cell);

    void setTimeout(int millis) {
        throw std::logic_error("unimplemented feature");
    }
    std::string send(boost::asio::serial_port& port, const std::string& msg);
protected:
    std::set<int> readCellNums();
    CellStats readCellStats(const std::set<int>& cellNums);
    void writeWord(int cell, int addr, uint16_t word);
    void writeByte(int cell, int addr, uint8_t byte);
    uint16_t readWord(int cell, int addr);
    uint8_t readByte(int cell, int addr);

    uint16_t volt2code(int cell, int volt);
    int code2volt(int cell, uint16_t code);

    uint16_t amp2code(int cell, int amp);
    int code2amp(int cell, uint16_t code);

    uint16_t speed2code(int cell, int speed);
    int code2speed(int cell, uint16_t code);
private:
    boost::asio::io_service  mIoService;
    boost::asio::serial_port mPort;

    CellStats mCellStats;
    std::mutex mMutex;
};

std::ostream& operator<<(std::ostream& stream, Amplifier::Stat stat);
