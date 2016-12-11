#include "emisstdc.hpp"

#include <iostream>

using std::logic_error;
using std::runtime_error;
using std::string;
using std::vector;

EmissTdc::EmissTdc(const string& name)
    : mEM1(0170000),
      mEM8({0x86, 5000, 0}),
      mName(name) {
    mBuffer.reserve(16*1024*1024);
}

EmissTdc::~EmissTdc() {
    if(isOpen())
        close();
}

void EmissTdc::open() {
    if(mEM1.isOpen() && mEM8.isOpen())
        throw logic_error("EmissTdc::open device is opened");
    mEM1.open("/dev/pq");
    try {
        mEM8.open(0x04b4, 0x1002);
    } catch(std::exception& e) {
        mEM1.close();
        throw e;
    }
    reset();
}

void EmissTdc::close() {
    if(!mEM1.isOpen() || !mEM8.isOpen())
        throw logic_error("EmissTdc::open device is closed");
    mEM1.resetSignal(1);
    mEM1.close();
    mEM8.close();
}

bool EmissTdc::isOpen() const {
    if(mEM1.isOpen() != mEM8.isOpen())
        throw runtime_error("EmissTdc::isOpen inconsistent state");
    return mEM1.isOpen() && mEM8.isOpen();
}

const string& EmissTdc::name() const {
    return mName;
}

void EmissTdc::readEvents(vector<EventHits>& buffer)  {
    buffer.clear();
    mEM1.resetSignal(1);
    mBuffer.resize(16*1024*1024);
    auto transfered = mEM8.readData(mBuffer);
    if(transfered > 4*1024*1024)
        throw runtime_error("EmissTdc::readEvents buffer overflow");
    mBuffer.resize(transfered);

    mEM1.generateSignal(ContrEM1::TypeSignal::pulse, 0);
    mEM1.generateSignal(ContrEM1::TypeSignal::potential, 1);

    EventHits event;
    for(size_t i = 0; i < mBuffer.size(); ++i) {
        if(mBuffer.at(i) != 0xFFFFFFFF) {
            continue;
        }
        buffer.emplace_back();
        
        for(size_t j = i+5; j < mBuffer.size() && mBuffer.at(j) != 0xFFFFFFFF; ++j) {
            auto word = uint16_t(mBuffer.at(j)&0xFFFF);
            if((word >> 15) == 0) {
                auto module = uint16_t((mBuffer.at(j)>>16)&0x3F);
                auto chan = (word >> 11);
                auto time = word & 0x3FF;
                buffer.back().emplace_back(EdgeDetection::leading, chan + module*32, time*7.8125);
            }
            /*
              TODO 
              ((word >> 14) == 0b10)
              ((word >> 14) == 0b11)
            */
            i = j;
        }
    }
}

void EmissTdc::readHits(vector<Hit>& buffer)  {
    throw runtime_error("Not implemented");
}

Tdc::Settings EmissTdc::settings()  {
    return Settings{
        8192,
        -8192,
        EdgeDetection::leading,
        8000,
     };
}

void EmissTdc::clear()  {
    mEM1.generateSignal(ContrEM1::TypeSignal::pulse, 0);
}

void EmissTdc::reset() {
    mEM1.resetQbus();
    mEM1.setAR();
    clear();
    mEM1.generateSignal(ContrEM1::TypeSignal::potential, 1);
}

Tdc::Mode EmissTdc::mode()  {
    return Mode::trigger;
}

void EmissTdc::setMode(Mode mode)  {
    if(mode != Mode::trigger)
        throw logic_error("EmissTdc::setMode only trigger mode supported");
}
