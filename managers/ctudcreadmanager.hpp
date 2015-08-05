#ifndef URAGANREADMANAGER_HPP
#define URAGANREADMANAGER_HPP

#include <list>
#include <tdcdata/ctudcrecord.hpp>
#include <cppchannel/channel>


#include "net/packagereciever.hpp"
#include "readmanager.hpp"

#include "threadblocker.hpp"

namespace caen {

class CtudcReadManager : public ReadManager {
  protected:
	using ByteVector      = std::vector<char>;
	using NevodPkgPtr     = std::unique_ptr<tdcdata::NevodPackage>;
	using DecorPkgPtr     = std::unique_ptr<tdcdata::DecorPackage>;
	using DataChannel     = cpp::ichannel<std::vector<char>>;
	using SystemClock     = std::chrono::high_resolution_clock;
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
  protected:
	bool init() override;
	void shutDown() override;
	void workerFunc() override;

	void handleDataPackages(WordVector& tdcData);

	void handleDecorPackage(ByteVector& buffer);
	void handleNevodPackage(ByteVector& buffer);

	void waitForDecorPackage();
	void waitForNevodPackage(SystemClock::time_point startTime);

	void writeCtudcRecord(const tdcdata::CtudcRecord& record);

  private:
	void outputEvents();
	PackageReciever mDecorReciever;
	PackageReciever mNevodReciever;

	DecorPkgPtr mDecorPackage;
	NevodPkgPtr mNevodPackage;

	DataChannel mDecorChannel;
	DataChannel mNevodChannel;
};

} //caen

#endif // URAGANREADMANAGER_HPP
