#pragma once

#include "net/packagereceiver.hpp"
#include "eventencoder.hpp"
#include "processmanager.hpp"

#include <json.hpp>
#include <atomic>

class ReadManager : public ProcessManager {
protected:
	using ModulePtr       = std::shared_ptr<Tdc>;
	using SystemClock     = std::chrono::high_resolution_clock;
	using TimePoint       = SystemClock::time_point;
	using RawEvent    = Tdc::EventHits;
	using RawEvents   = std::vector<RawEvent>;
public:
	struct NetInfo {
		std::string nevodIP;
		uint16_t    nevodPort;
	};
	struct Settings {
		unsigned    nRun;
		unsigned    eventsPerFile;
		std::string writeDir;
		std::string infoIp;
		uint16_t    infoPort;

		nlohmann::json marshal() const;
		void unMarshal(const nlohmann::json& doc);
	};
public:
	ReadManager(ModulePtr module,
                const Settings& settings,
				const ChannelConfig& config);
	~ReadManager();
	void run() override;
	void stop() override;
	double getTriggerFrequency() const;
	double getPackageFrequency() const;
protected:
	void increasePackageCount();
	void increaseTriggerCount(uintmax_t val);
	void resetPackageCount();
	void resetTriggerCount();
	void handleNevodPackage(PackageReceiver::ByteVector& buffer, trek::data::NevodPackage& nvdPkg);
	static std::string formDir(const Settings& settings);
	void outputMeta(const std::string& dirName, const Settings& settings, Tdc& module);
private:
	ModulePtr                mModule;
	EventEncoder             mEncoder;
	PackageReceiver          mNevodReceiver;
	trek::data::NevodPackage mNevodPackage;
	RawEvents                mBuffer;

	std::atomic<uintmax_t> mPackageCount;
	std::atomic<uintmax_t> mTriggerCount;

	TimePoint mStartPoint;
};
