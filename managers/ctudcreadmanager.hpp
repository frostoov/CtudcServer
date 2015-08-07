#ifndef URAGANREADMANAGER_HPP
#define URAGANREADMANAGER_HPP

#include <tdcdata/ctudcrecord.hpp>

#include "net/packagereceiver.hpp"
#include "readmanager.hpp"

namespace caen {

class CtudcReadManager : public ReadManager {
  protected:
	using NevodPkgPtr     = std::unique_ptr<tdcdata::NevodPackage>;
	using SystemClock     = std::chrono::high_resolution_clock;
	using TimePoint       = SystemClock::time_point;
  public:
	struct NetInfo {
		std::string nevodIP;
		uint16_t    nevodPort;
	};
  public:
	CtudcReadManager(ModulePtr module, const std::string& path, size_t eventNum,
					 const ChannelConfig& config, const NetInfo& netInfo);
	bool start() override;
	void stop() override;
	double getTriggerFrequency() const;
	double getPackageFrequency() const;
  protected:
	bool init() override;
	void workerFunc() override;

	void handleDataPackages(WordVector& tdcData);
	void handleNevodPackage(PackageReceiver::ByteVector& buffer);
	void writeCtudcRecord(const tdcdata::CtudcRecord& record);
  private:
	PackageReceiver mNevodReciever;
	NevodPkgPtr mNevodPackage;

	uintmax_t mPackageCount;
	uintmax_t mTriggerCount;

	TimePoint mStartPoint;
	bool mNetIsActive;
};

} //caen

#endif // URAGANREADMANAGER_HPP
