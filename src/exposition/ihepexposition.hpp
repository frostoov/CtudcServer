#pragma once

#include "exposition.hpp"

class IHEPExposition : public Exposition {
	using EventBuffer = std::vector<Tdc::EventHits>;
public:
	struct Settings {
		uintmax_t nRun;
		uintmax_t eventsPerFile;
		std::string writeDir;

		uintmax_t readFreq;
	};
public:
	IHEPExposition(std::shared_ptr<Tdc> tdc,
				   const Settings& settings,
				   const ChannelConfig& config);
	~IHEPExposition();
	operator bool() const override { return mActive; }
	
	void stop() override;
	
	uintmax_t triggerCount() const override { return mTrgCount; }
	uintmax_t triggerDrop() const override { return 0; }
	
	uintmax_t packageCount() const override { return 0; }
	uintmax_t packageDrop() const override { return 0; }
	
	TrekHitCount chambersCount() const override { return mChambersCount; }
	TrekHitCount chambersDrop() const override { return TrekHitCount(); }

    TimePoint startPoint() const override { return mStartPoint; }
protected:
	void readLoop(std::shared_ptr<Tdc> tdc, const Settings& settings, const ChannelConfig chanConf);
	std::vector<trek::data::EventHits> handleEvents(const EventBuffer& buffer, const ChannelConfig& conf);
private:
	std::thread mReadThread;
	
	uintmax_t mTrgCount;
	TrekHitCount mChambersCount;
	
	std::atomic_bool mActive;
	std::condition_variable mCv;

    TimePoint mStartPoint = Clock::now();
};




