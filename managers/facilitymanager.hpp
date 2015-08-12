#ifndef DEVICEMANAGER_HPP
#define DEVICEMANAGER_HPP

#include <mutex>
#include <json.hpp>


#include "caen/tdcmodule.hpp"
#include "processmanager.hpp"
#include "frequencymanager.hpp"

class FacilitySettings {
public:
	FacilitySettings() = default;

	nlohmann::json marshal() const;
	void unMarshal (const nlohmann::json& doc);

	int32_t getVmeAddress() const;
	uint64_t getNumberOfRun() const;
	uintmax_t getEventsPerFile() const;
	const std::string& getWriteDir() const;
	const std::string& getInfoPkgIp() const;
	uint16_t getInfoPkgPort() const;

	void setVmeAddress (int32_t vmeAddress);
	void setNumberOfRun (uint64_t numberOfRun);
	void setEventsPerFile (uintmax_t eventsPerFile);
	void setWriteDir (const std::string& dir);
	void setInfoPkgIp (const std::string& ip);
	void setInfoPkgPort (uint16_t port);

private:
	int32_t     mVmeAddress;
	uint64_t    mNumberOfRun;
	uintmax_t   mEventsPerFile;
	std::string mWriteDir;
	std::string mInfoPkgIp;
	uint16_t    mInfoPkgPort;
};

class FacilityManager {
	using Mutex = std::mutex;
	using Lock = std::lock_guard<Mutex>;
	using TdcModulePtr = std::shared_ptr<caen::Module>;
	using ProcessManagerPtr = std::unique_ptr<caen::ProcessManager>;
	struct Query {
		nlohmann::json::string_t  procedure;
		nlohmann::json::array_t   input;
	};
	struct Response {
		nlohmann::json::string_t  procedure;
		nlohmann::json::array_t   output;
		nlohmann::json::boolean_t status;
	};
	using Procedure = std::function<Response (const Query&) >;
	using Procedures = std::unordered_map<std::string, Procedure>;
public:
	using Callback = std::function<void (const FacilityManager&) >;
public:
	FacilityManager (int32_t vmeAddress, const caen::ChannelConfig& channelConfig,
	                 const FacilitySettings& settings);
	std::string handleQuery (const std::string& rawQuery);
	void setStopReadCallback (Callback&& callback);
	const FacilitySettings& getSettings() const;
protected:
	Procedures createProcedures();

	static Query convertQuery (const std::string& rawQuery);
	static std::string convertResponse (const Response& response);
	Procedure getProcedure (const Query& query) const;

	Response init (const Query& query);
	Response close (const Query& query);
	Response isInit (const Query& query);
	Response setLog (const Query& query);
	Response getLog (const Query& query);
	Response getSettings (const Query& query);
	Response updateSettings (const Query& query);
	Response setSettings (const Query& query);
	Response setTriggerMode (const Query& query);
	Response setTriggerSubtraction (const Query& query);
	Response setTdcMeta (const Query& query);
	Response setWindowWidth (const Query& query);
	Response setWindowOffset (const Query& query);
	Response setEdgeDetection (const Query& query);
	Response setLsb (const Query& query);
	Response setAlmostFull (const Query& query);
	Response setControl (const Query& query);
	Response setDeadTime (const Query& query);
	Response setEventBLT (const Query& query);
	Response getProcess (const Query& query);
	Response startRead (const Query& query);
	Response stopRead (const Query& query);
	Response startFrequency (const Query& query);
	Response stopFrequency (const Query& query);

	Response getCurrentRun (const Query& query);

	Response getTriggerFrequency (const Query& query) const;
	Response getPackageFrequency (const Query& query) const;

	ProcessManagerPtr createReadManager (const Query& query) const;
	bool isReadManager (const ProcessManagerPtr& processManager) const;
	bool isFreqManager (const ProcessManagerPtr& processManager) const;
	bool isCtudcReadManager (const ProcessManagerPtr& processManager) const;
	nlohmann::json::array_t convertFreq (const caen::TrekFrequency& freq) const;
	nlohmann::json::array_t getProcessType (const ProcessManagerPtr& mProcess) const;
	trekdata::Settings createSettings (const Query& query) const;
	std::string createWriteDir() const;

	void stopReadCallback();
private:
	TdcModulePtr        mTdcModule;
	ProcessManagerPtr   mProcess;
	caen::ChannelConfig mChannelConfig;
	FacilitySettings    mSettings;

	Procedures mProcedures;
	Mutex      mMutex;
	Callback   mStopReadCallback;
};

#endif // DEVICEMANAGER_HPP
