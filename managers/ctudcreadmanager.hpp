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
	using SystemClock     = std::chrono::high_resolution_clock;
	using DecorPackages   = std::list<tdcdata::DecorPackage>;
	using NevodPackages   = std::list<tdcdata::NevodPackage>;
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

	~CtudcReadManager();
	const char* getTitle() const override {return "UraganReadManager";}
  protected:
	void workerLoop() override;
	void stop() override;

	void handleDataPackages(WordVector& tdcData);

	void handleDecorPackage(char* data, size_t size);
	void handleNevodPackage(char* data, size_t size);

	void waitForDecorPackage();
	void waitForNevodPackage(SystemClock::time_point startTime);

	void writeCtudcRecord(const tdcdata::CtudcRecord& record);

	bool isHandling() const {return mIsHandling;}

  private:
	void outputEvents();
	PackageReciever mDecorReciever;
	PackageReciever mNevodReciever;

	DecorPackages mDecorPackages;
	NevodPackages mNevodPackages;

	ThreadBlocker mDecorBlocker;
	ThreadBlocker mNevodBlocker;

	volatile bool mIsHandling;
};

} //caen

#endif // URAGANREADMANAGER_HPP
