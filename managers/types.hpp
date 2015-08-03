#ifndef TYPES
#define TYPES

#include <cstdint>
#include <unordered_map>

namespace caen {

class ChannelCongruence {
  public:
	ChannelCongruence(uintmax_t chamber, uintmax_t wire)
		: mChamber(chamber), mWire(wire) { }

	uintmax_t getChamber() const {return mChamber;}
	uintmax_t getWire() const {return mWire;}
  private:
	uintmax_t mChamber;
	uintmax_t mWire;
};

using ChannelConfig = std::unordered_map<uintmax_t, ChannelCongruence>;

}

#endif // TYPES

