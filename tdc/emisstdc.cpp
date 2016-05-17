#include "emisstdc.hpp"

#include <gsl/gsl_util.h>
#include <iostream>

using std::logic_error;
using std::runtime_error;
using std::string;
using std::vector;

EmissTdc::EmissTdc()
    : mEM1(0170000),
      mEM8({0x86, 5000, 0}) {
    mBuffer.reserve(16*1024*1024);
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
    mEM1.resetQbus();
    mEM1.setAR();
    mEM1.generateSignal(ContrEM1::TypeSignal::pulse, 0);
    mEM1.generateSignal(ContrEM1::TypeSignal::potential, 1);
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

uint16_t EmissTdc::stat() {
    return mEM1.readStatus();
}

const string& EmissTdc::name() const {
    static string n("E-Miss TDC");
    return n;
}

void EmissTdc::readEvents(vector<EventHits>& buffer)  {
    buffer.clear();
    mEM1.resetSignal(1);
    auto f = gsl::finally([&]{
        mEM1.generateSignal(ContrEM1::TypeSignal::pulse, 0);
        mEM1.generateSignal(ContrEM1::TypeSignal::potential, 1);
    });
    mBuffer.resize(16*1024*1024);
    auto transfered = mEM8.readData(mBuffer);
    if(transfered >= 4*1024*1024)
        throw runtime_error("EmissTdc::readEvents buffer overflow");
    std::cout << "transfered: " << transfered << '\n';
    mBuffer.resize(transfered);

    size_t start = 0;
    for(size_t j = 0; j < mBuffer.size(); ++j) {
        if(mBuffer.at(j) == 0xFFFFFFFF) {
            start = j;
            break;
        }
    }
    for(size_t i = start; i < mBuffer.size(); ++i) {
        if(mBuffer.at(i) == 0xFFFFFFFF) {
            i += 5;
            buffer.emplace_back();
        }
        uint32_t module;
        uint32_t word;
        while(i < mBuffer.size()) {
            if(mBuffer.at(i) == 0xFFFFFFFF) {
                --i;
                break;
            }
            word = uint16_t(mBuffer.at(i)&0xFFFF);
            module = uint16_t((mBuffer.at(i)>>16)&0x3F);
            if((word >> 15) == 0) {
                auto chan = (word >> 10) + 32*module;
                auto time = word & 0x3FF;
                buffer.back().emplace_back(EdgeDetection::leading, chan + module*32, 8*time);
            } if((word >> 14) == 0b10) {
                //TODO
            } else if((word >> 14) == 0b11) {
                //TODO
            }
            ++i;
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
    //mEM1.generateSignal(ContrEM1::TypeSignal::pulse, 0);
}

Tdc::Mode EmissTdc::mode()  {
    return Mode::trigger;
}

void EmissTdc::setMode(Mode mode)  {
    if(mode != Mode::trigger)
        throw logic_error("EmissTdc::setMode only trigger mode supported");
}
