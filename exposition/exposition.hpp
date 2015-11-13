#pragma once

#include "eventwriter.hpp"
#include "process.hpp"

#include <json.hpp>
#include <atomic>

class Exposition : public ProcessManager {
protected:
	using ModulePtr   = std::shared_ptr<Tdc>;
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
	Exposition(ModulePtr module,
	           const Settings& settings,
	           const ChannelConfig& config);
	~Exposition();
	void run() override;
	void stop() override;
	uintmax_t triggerCount() const;
	uintmax_t packageCount() const;
	uintmax_t duration() const;
protected:
	void increasePackageCount();
	void increaseTriggerCount(uintmax_t val);
	void resetPackageCount();
	void resetTriggerCount();
	static std::string formDir(const Settings& settings);
	static std::string formPrefix(const Settings& settings);
	void outputMeta(const std::string& dirName, const Settings& settings, Tdc& module);
private:
	ModulePtr                mModule;
	EventWriter              mEncoder;
	RawEvents                mBuffer;

	std::atomic<bool>      mWorking;
	std::atomic<uintmax_t> mPackageCount;
	std::atomic<uintmax_t> mTriggerCount;

	std::chrono::system_clock::time_point mStartPoint;
};
