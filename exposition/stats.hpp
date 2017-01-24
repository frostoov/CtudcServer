#pragma once

#include "channelconfig.hpp"

#include <trek/data/eventrecord.hpp>

#include <vector>

class Statistics {
public:
	void incrementTriggers(uintmax_t d, bool matched) {
		mTrgCount[matched ? 0 : 1] += d;
	}

	void incrementPackages(uintmax_t d, bool matched) {
		mPkgCount[matched ? 0 : 1] += d;
	}

	void incrementChambersCount(const std::vector<trek::data::EventRecord>& records, bool matched) {
		int i = matched ? 1 : 0;
		
		for(auto& event : records) {
			for(auto& hit : event) {
				if(mChambersCount[i].count(hit.chamber()) == 0)
				   mChambersCount[i].emplace(hit.chamber(), ChamberHitCount{{0, 0, 0, 0}});
				++mChambersCount[i].at(hit.chamber()).at(hit.wire());
			}
		}
	}

	uintmax_t triggerCount() const {
		return mTrgCount[0];
	}

	uintmax_t triggerDrops() const {
		return mTrgCount[1];
	}

	uintmax_t packageCount() const {
		return mPkgCount[0];
	}

	uintmax_t packageDrops() const {
		return mPkgCount[1];
	}

	TrekHitCount chambersCount() const {
		return mChambersCount[0];
	}

	TrekHitCount chambersDrops() const {
		return mChambersCount[1];
	}
private:
    uintmax_t mTrgCount[2] = {0, 0};
    
    uintmax_t mPkgCount[2] = {0, 0};
    
    TrekHitCount mChambersCount[2];
};
