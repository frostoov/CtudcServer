#ifndef URAGANREADMANAGER_HPP
#define URAGANREADMANAGER_HPP

#include <list>
#include <tdcdata/ctudcrecord.hpp>

#include "net/packagereciever.hpp"
#include "readmanager.hpp"

#include "threadblocker.hpp"

namespace caen {

class CtudcReadManager : public ReadManager {
  protected:
	using ByteVector      = std::vector<char>;
	using NevodPkgPtr     = std::unique_ptr<tdcdata::NevodPackage>;
	using DecorPkgPtr     = std::unique_ptr<tdcdata::DecorPackage>;
	using DataChannel     = Channel<std::vector<char>>;
	using SystemClock     = std::chrono::high_resolution_clock;
	using TimePoint       = SystemClock::time_point;
  public:
	struct NetInfo {
		std::string decorIP;
		uint16_t    decorPort;
		std::string nevodIP;
		uint16_t    nevodPort;
	};
  public:
	CtudcReadManager(ModulePtr module, const std::string& path, size_t eventNum,
					 const ChannelConfig& config, const NetInfo& netInfo);
	uintmax_t getTriggerCount() const;
	uintmax_t getPackageCount() const;
	double getTriggerFrequency() const;
	double getPackageFrequency() const;
  protected:
	bool init() override;
	void shutDown() override;
	void workerFunc() override;

	void handleDataPackages(WordVector& tdcData);

	void handleDecorPackage(ByteVector&& buffer);
	void handleNevodPackage(ByteVector&& buffer);

	void waitForDecorPackage();
	void waitForNevodPackage(SystemClock::time_point startTime);

	void writeCtudcRecord(const tdcdata::CtudcRecord& record);

  private:
	void outputEvents();
	PackageReceiver mDecorReciever;
	PackageReceiver mNevodReciever;

	DecorPkgPtr mDecorPackage;
	NevodPkgPtr mNevodPackage;

	DataChannel& mDecorChannel;
	DataChannel& mNevodChannel;

	uintmax_t mPackageCount;
	uintmax_t mTriggerCount;

	TimePoint mStartPoint;
};

} //caen

#endif // URAGANREADMANAGER_HPP
