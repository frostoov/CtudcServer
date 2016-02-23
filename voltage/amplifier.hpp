#pragma once

#include "serialbuf.hpp"
#include <iostream>
#include <map>
#include <set>

class Amplifier {
    struct CellStat {
        uint16_t umesmax;
        uint16_t umax;
        uint16_t umin;

        uint16_t imesmax;
        uint16_t imax;
    };
    using CellStats = std::map<int, CellStat>;
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
    private:
        uint8_t mStat;
    };
public:
    Amplifier();

	void open(const std::string& name);
	void close();
	bool isOpen() const { return mBuffer.isOpen(); }

    Stat stat(int cell);

	void setVoltage(int cell, double code);
	void setAmperage(int cell, double amp);

    void setSpeedUp(int cell, uint8_t code);
    void setSpeedDn(int cell, uint8_t code);

    double speedUp(int cell);
    double speedDn(int cell);

    double voltage(int cell);
    double amperage(int cell);

    void turnOn(int cell);
    void turnOff(int cell);

    void setTimeout(int millis) {mBuffer.setTimeout(millis);}
protected:
    std::set<int> readCellNums();
    CellStats readCellStats(const std::set<int>& cellNums);
    void writeWord(int cell, int addr, uint16_t word);
    void writeByte(int cell, int addr, uint8_t byte);
    uint16_t readWord(int cell, int addr);
    uint8_t readByte(int cell, int addr);

    uint16_t volt2code(int cell, double volt);
    double code2volt(int cell, uint16_t code);

    uint16_t amp2code(int cell, double amp);
    double code2amp(int cell, uint16_t code);

    uint16_t speed2code(int cell, double speed);
    double code2speed(int cell, uint16_t code);

private:
	serialbuf mBuffer;
	std::iostream mStream;

    CellStats mCellStats;
};

std::ostream& operator<<(std::ostream& stream, Amplifier::Stat stat);
