#include "emissem1.hpp"

#include <stdexcept>

using std::runtime_error;
using std::string;
using std::vector;

enum class EmissEM1::Reg: uint16_t {
    status      = 00,
    control     = 06,
    pchiStatus  = 012,
    pchiAddress = 014,
    pchiData    = 016
};

class EmissEM1::PchiStat {
public:
    PchiStat() : mStat(0) { }
    PchiStat(uint16_t stat) : mStat(stat) { }
    PchiStat& operator=(uint16_t stat) {
        mStat = stat;
        return *this;
    }

    bool stop() const {
        return ((mStat >> 14) & 1) == 1;
    }
    bool error() const {
        return ((mStat >> 15) & 1) == 1;
    }
    bool overflow() const {
        return ((mStat >> 13) & 1) == 1;
    }
    size_t size() const {
        return mStat & 0b11111111111;
    }
private:
    uint16_t mStat;
};

class EmissEM1::Decoder {
public:
    static void decode(const uint16_t* data, size_t size, vector<EventHits >& buffer) {
        size_t i = 0;
        while(i < size) {
            if( (data[i] & 0x8000) != 1)
                throw runtime_error("EmissEM1::Decoder::decoder invalid heaeder");
            auto mod = module(data[i]);
            auto fw = fifoWords(data[i]);
            decodeModule(mod, data+i, fw, buffer);
            i += fw;
        }
    }
protected:
    static void decodeModule(unsigned mod, const uint16_t* data, size_t size, vector<EventHits >& buffer) {
        size_t evtCounter = 0;
        for(size_t j = 0; j < size; ++j) {
            if( (data[j] & 0xC000) != 0x3) {
                if(buffer.size() < evtCounter+1) {
                    buffer.resize(evtCounter+1);
                }
                buffer.at(evtCounter).push_back({16*mod + chan(data[j]), unsigned(7812.5*time(data[j]))});
            } else
                ++evtCounter;
        }
    }
    static unsigned module(uint16_t word)      {return unsigned((word >> 9) & 0b111111);}
    static unsigned fifoWords(uint16_t word)   {return unsigned(word & 0b111111111);}
    static unsigned chan(uint16_t word)        {return unsigned((word >> 10) & 0b11111); }
    static unsigned time(uint16_t word)        {return unsigned(word & 0b1111111111);}
    static unsigned eventNumber(uint16_t word) {return unsigned(word & 0b11111111111111);}
};

EmissEM1::EmissEM1(long address)
    : mAddress(address) {  }

void EmissEM1::read(vector<EventHits>& buffer) {
    buffer.clear();
    static uint16_t buf[1024];
    setPchi();
    PchiStat pchiStat;
    do {
        pchiStat = mDev.readWord(mAddress + long(Reg::pchiStatus));
    } while(!pchiStat.stop() && !pchiStat.overflow());
    if(pchiStat.error())
        throw runtime_error("EmissEM1::read pchi error");
    do {
        auto transfered = mDev.read(mAddress + long(Reg::pchiData), buf, pchiStat.size());
        pchiStat = mDev.readWord(mAddress + long(Reg::pchiStatus));
        Decoder::decode(buf, transfered, buffer);
    } while(!pchiStat.stop());
}

void EmissEM1::clear() {
    mDev.writeWord(mAddress + long(Reg::status), 0b1000000000000000);
}

void EmissEM1::open(const string& devName) {
    mDev.open(devName);
}

void EmissEM1::close() {
    mDev.close();
}

const string& EmissEM1::name() const {
    static const string n("EmissEm1");
    return n;
}

EmissEM1::Settings EmissEM1::settings() {
    return {
        8000,
        -8000,
        EdgeDetection::leading,
        7812,
    };
}

bool EmissEM1::isOpen() const {
    return mDev.isOpen();
}

uint8_t EmissEM1::ctrl() {
    return mDev.readWord(mAddress + long(Reg::control));
}

uint16_t EmissEM1::stat() {
    return mDev.readWord(mAddress + long(Reg::status));
}

void EmissEM1::setPchi() {
    mDev.writeWord(mAddress + long(Reg::control), 0b10000000);
}
