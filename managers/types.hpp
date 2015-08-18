#ifndef TYPES
#define TYPES

#include <cstdint>
#include <unordered_map>

namespace caen {

class ChannelCongruence {
public:
    ChannelCongruence(uint32_t chamber, uint32_t wire)
        : mChamber(chamber), mWire(wire) { }

    uint32_t getChamber() const {
        return mChamber;
    }
    uint32_t getWire() const {
        return mWire;
    }
private:
    uint32_t mChamber;
    uint32_t mWire;
};

using ChannelConfig = std::unordered_map<uint32_t, ChannelCongruence>;

}

#endif // TYPES

