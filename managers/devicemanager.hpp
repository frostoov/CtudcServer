#ifndef DEVICEMANAGER_HPP
#define DEVICEMANAGER_HPP

#include <json.hpp>


#include "caen/tdcmodule.hpp"
#include "processmanager.hpp"
#include "frequencymanager.hpp"

class DeviceManager {
	using DevicePtr = std::shared_ptr<caen::Module>;
	using ProcessManagerPtr = std::shared_ptr<caen::ProcessManager>;
	struct Query {
		nlohmann::json::string_t  procedure;
		nlohmann::json::array_t   input;
	};
	struct Response {
		nlohmann::json::string_t  procedure;
		nlohmann::json::array_t   output;
		nlohmann::json::boolean_t status;
	};
	using Procedure = std::function<Response(const Query&)>;
	using Procedures = std::unordered_map<std::string, Procedure>;
  public:
	DeviceManager(int32_t vmeAddress, const caen::ChannelConfig& channelConfig);
	std::string handleQuery(const std::string& rawQuery);
  protected:
	Procedures createProcedures();

	static Query convertQuery(const std::string& rawQuery);
	static std::string convertResponse(const Response& response);
	Procedure getProcedure(const Query& query) const;

	Response init(const Query& query);
	Response close(const Query& query);
	Response isInit(const Query& query);
	Response setLog(const Query& query);
	Response getLog(const Query& query);
	Response getSettings(const Query& query);
	Response updateSettings(const Query& query);
	Response setSettings(const Query& query);
	Response setTriggerMode(const Query& query);
	Response setTriggerSubtraction(const Query& query);
	Response setTdcMeta(const Query& query);
	Response setWindowWidth(const Query& query);
	Response setWindowOffset(const Query& query);
	Response setEdgeDetection(const Query& query);
	Response setLsb(const Query& query);
	Response setAlmostFull(const Query& query);
	Response setControl(const Query& query);
	Response setDeadTime(const Query& query);
	Response setEventBLT(const Query& query);
	Response getProcess(const Query& query);
	Response startRead(const Query& query);
	Response stopRead(const Query& query);
	Response startFrequency(const Query& query);
	Response stopFrequency(const Query& query);

	ProcessManagerPtr createReadManager(const Query& query);
	bool isReadManager(const ProcessManagerPtr& mProcessManager);
	bool isFreqManager(const ProcessManagerPtr& mProcessManager);
	nlohmann::json::array_t convertFreq(const caen::TrekFrequency& freq);
	nlohmann::json::array_t getProcessType(const ProcessManagerPtr& mProcessManager);

  private:
	DevicePtr mDevice;
	ProcessManagerPtr mProcessManager;
	caen::ChannelConfig mChannelConfig;
	size_t eventsPerFile;

	Procedures mProcedures;
};

#endif // DEVICEMANAGER_HPP
