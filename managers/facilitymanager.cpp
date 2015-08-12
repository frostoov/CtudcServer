#include <trekdata/tdcsettings.hpp>
#include <sstream>
#include <typeinfo>
#include <iomanip>

#include "facilitymanager.hpp"
#include "readmanager.hpp"
#include "ctudcreadmanager.hpp"
#include "frequencymanager.hpp"


using std::cout;
using std::endl;
using std::setw;
using std::setfill;
using std::ostringstream;

using std::string;
using std::make_unique;
using std::chrono::microseconds;

using nlohmann::json;

using trekdata::EdgeDetection;
using trekdata::Lsb;

using caen::ReadManager;
using caen::CtudcReadManager;
using caen::FrequencyManager;

FacilityManager::FacilityManager (int32_t vmeAddress,
                                  const caen::ChannelConfig& channelConfig,
                                  const FacilitySettings& settings)
	: mTdcModule (make_unique<caen::Module> (vmeAddress) ),
	  mChannelConfig (channelConfig),
	  mSettings (settings),
	  mProcedures (createProcedures() ) { }

FacilityManager::Procedures FacilityManager::createProcedures() {
	return {
		{"init",                   [&] (const Query & query) { return this->init (query);}},
		{"close",                  [&] (const Query & query) { return this->close (query);}},
		{"isInit",                 [&] (const Query & query) { return this->isInit (query);}},
		{"getSettings",            [&] (const Query & query) { return this->getSettings (query);}},
		{"setSettings",            [&] (const Query & query) { return this->setSettings (query);}},
		{"updateSettings",         [&] (const Query & query) { return this->updateSettings (query);}},
		{"getLog",                 [&] (const Query & query) { return this->getLog (query);}},
		{"setLog",                 [&] (const Query & query) { return this->setLog (query);}},
		{"setTriggerMode",	       [&] (const Query & query) { return this->setTriggerMode (query);}},
		{"setTriggerSubtraction",  [&] (const Query & query) { return this->setTriggerSubtraction (query);}},
		{"setTdcMeta",             [&] (const Query & query) { return this->setTdcMeta (query);}},
		{"setWindowWidth",         [&] (const Query & query) { return this->setWindowWidth (query);}},
		{"setWindowOffset",        [&] (const Query & query) { return this->setWindowOffset (query);}},
		{"setEdgeDetection",       [&] (const Query & query) { return this->setEdgeDetection (query);}},
		{"setLsb",                 [&] (const Query & query) { return this->setLsb (query);}},
		{"setAlmostFull",          [&] (const Query & query) { return this->setAlmostFull (query);}},
		{"setControl",             [&] (const Query & query) { return this->setControl (query);}},
		{"setDeadTime",            [&] (const Query & query) { return this->setDeadTime (query);}},
		{"setEventBLT",            [&] (const Query & query) { return this->setEventBLT (query);}},
		{"getProcess",             [&] (const Query & query) { return this->getProcess (query);} },
		{"startRead",              [&] (const Query & query) { return this->startRead (query);} },
		{"stopRead",               [&] (const Query & query) { return this->stopRead (query);} },
		{"startFrequency",         [&] (const Query & query) { return this->startFrequency (query);} },
		{"stopFrequency",          [&] (const Query & query) { return this->stopFrequency (query);} },
		{"getTriggerFrequency",    [&] (const Query & query) { return this->getTriggerFrequency (query);} },
		{"getPackageFrequency",    [&] (const Query & query) { return this->getPackageFrequency (query);} },
		{"getCurrentRun",          [&] (const Query & query) { return this->getCurrentRun (query); } },
	};
}

string FacilityManager::handleQuery (const string& rawQuery) {
	Lock lock (mMutex);
	auto query = convertQuery (rawQuery);
	auto procedure = getProcedure (query);
	auto response = procedure (query);
	return convertResponse (response);
}

void FacilityManager::setStopReadCallback (FacilityManager::Callback&& callback) {
	mStopReadCallback = std::move (callback);
}

FacilityManager::Query FacilityManager::convertQuery (const string& rawQuery) {
	auto query = json::parse (rawQuery);
	return {
		query.at ("procedure"),
		query.at ("input"),
	};
}

string FacilityManager::convertResponse (const FacilityManager::Response& response) {
	return json{
		{"procedure", response.procedure},
		{"output",    response.output},
		{"status",    response.status},
	} .dump();
}

FacilityManager::Procedure FacilityManager::getProcedure (const FacilityManager::Query& query) const {
	return mProcedures.at (query.procedure);
}

FacilityManager::Response FacilityManager::init (const Query& query) {
	return {
		"init",
		json::array(),
		mProcess ? false : mTdcModule->initialize(),
	};
}

FacilityManager::Response FacilityManager::close (const Query& query) {
	return {
		"close",
		json::array(),
		mProcess ? false : mTdcModule->close(),
	};
}

FacilityManager::Response FacilityManager::isInit (const Query& query) {
	return {
		"isInit",
		json::array ({mTdcModule->isInit() }),
		true,
	};
}

FacilityManager::Response FacilityManager::setLog (const Query& query) {
	mTdcModule->setLog (query.input.at (0) );
	return {
		"setLog",
		json::array(),
		true,
	};
}

FacilityManager::Response FacilityManager::getLog (const Query& query) {
	json::array_t log;
	while (mTdcModule->hasMessages() )
	{ log.push_back (mTdcModule->popMessage().first); }
	return {
		"getLog",
		log,
		true,
	};
}

FacilityManager::Response FacilityManager::getSettings (const Query& query) {
	const auto& settings = mTdcModule->getSettings();
	return {
		"getSettings",
		{
			settings.getTriggerMode(),
			settings.getTriggerSubtraction(),
			settings.getTdcMeta(),
			settings.getWindowWidth(),
			settings.getWindowOffset(),
			static_cast<uint16_t> (settings.getEdgeDetection() ),
			static_cast<uint16_t> (settings.getLsb() ),
			settings.getAlmostFull(),
			settings.getControl(),
			settings.getStatus(),
			settings.getDeadTime(),
			settings.getEventBLT()
		},
		true
	};
}

FacilityManager::Response FacilityManager::updateSettings (const Query& query) {
	return {
		"updateSettings",
		json::array(),
		mProcess ? false : mTdcModule->updateSettings(),
	};
}

FacilityManager::Response FacilityManager::setSettings (const Query& query) {
	auto settings  = createSettings (query);
	return {
		"setSettings",
		json::array(),
		mProcess ? false : mTdcModule->setSettings (settings),
	};
}

FacilityManager::Response FacilityManager::setTriggerMode (const Query& query) {
	return {
		"setTriggerMode",
		json::array(),
		mProcess ? false : mTdcModule->setTriggerMode (query.input.front() ),
	};
}

FacilityManager::Response FacilityManager::setTriggerSubtraction (const Query& query) {
	return {
		"setTriggerSubtraction",
		json::array(),
		mProcess ? false : mTdcModule->setTriggerSubtraction (query.input.front() ),
	};
}

FacilityManager::Response FacilityManager::setTdcMeta (const Query& query) {
	return {
		"setTdcMeta",
		json::array(),
		mProcess ? false : mTdcModule->setTdcMeta (query.input.front() ),
	};
}

FacilityManager::Response FacilityManager::setWindowWidth (const Query& query) {
	return {
		"setWindowWidth",
		json::array(),
		mProcess ? false : mTdcModule->setWindowWidth (query.input.front() ),
	};
}

FacilityManager::Response FacilityManager::setWindowOffset (const Query& query) {
	return {
		"setWindowOffset",
		json::array(),
		mProcess ? false : mTdcModule->setWindowOffset (query.input.front() ),
	};
}

FacilityManager::Response FacilityManager::setEdgeDetection (const Query& query) {
	auto edgeDetection = static_cast<uint16_t> (query.input.front() );
	return {
		"setEdgeDetection",
		json::array(),
		mProcess ? false : mTdcModule->setEdgeDetection (static_cast<EdgeDetection> (edgeDetection) ),
	};
}

FacilityManager::Response FacilityManager::setLsb (const Query& query) {
	auto lsb = static_cast<uint16_t> (query.input.front() );
	return {
		"setLsb",
		json::array(),
		mProcess ? false : mTdcModule->setLsb (static_cast<Lsb> (lsb) ),
	};
}

FacilityManager::Response FacilityManager::setAlmostFull (const Query& query) {
	return {
		"setAlmostFull",
		json::array(),
		mProcess ? false : mTdcModule->setAlmostFull (query.input.front() ),
	};
}

FacilityManager::Response FacilityManager::setControl (const Query& query) {
	return {
		"setControl",
		json::array(),
		mProcess ? false : mTdcModule->setControl (query.input.front() ),
	};
}

FacilityManager::Response FacilityManager::setDeadTime (const Query& query) {
	return {
		"setDeadTime",
		json::array(),
		mProcess ? false : mTdcModule->setDeadTime (query.input.front() ),
	};
}

FacilityManager::Response FacilityManager::setEventBLT (const Query& query) {
	return {
		"setEventBLT",
		json::array(),
		mProcess ? false : mTdcModule->setEventBLT (query.input.front() ),
	};
}

FacilityManager::Response FacilityManager::getProcess (const Query& query) {
	return {
		"getProcess",
		getProcessType (mProcess),
		true
	};
}

FacilityManager::Response FacilityManager::startRead (const Query& query) {
	auto status = false;
	if (!mProcess) {
		mProcess = createReadManager (query);
		status = mProcess->start();
		if (!status) { mProcess.reset(); }
	}
	return {
		"startRead",
		json::array(),
		status,
	};
}

FacilityManager::Response FacilityManager::stopRead (const Query& query) {
	auto status = false;
	if ( isReadManager (mProcess) || isCtudcReadManager (mProcess) ) {
		mProcess->stop();
		mProcess.reset();
		mSettings.setNumberOfRun (mSettings.getNumberOfRun() + 1);
		stopReadCallback();
		status = true;
	}
	return {
		"stopRead",
		json::array(),
		status
	};
}

FacilityManager::Response FacilityManager::startFrequency (const Query& query) {
	auto status = false;
	if (!mProcess) {
		mProcess = make_unique<FrequencyManager> (mTdcModule, mChannelConfig, microseconds (100) );
		status = mProcess->start();
		if (!status) { mProcess.reset(); }
	}
	return {
		"startFrequency",
		json::array(),
		status
	};
}

FacilityManager::Response FacilityManager::stopFrequency (const Query& query) {
	auto responseStatus = false;
	caen::TrekFrequency trekFreq;
	if (isFreqManager (mProcess) ) {
		mProcess->stop();
		auto freqManager = dynamic_cast<FrequencyManager*> (mProcess.get() );
		trekFreq = freqManager->getFrequency();
		mProcess.reset();
		responseStatus = true;
	}
	return {
		"stopFrequency",
		convertFreq (trekFreq),
		responseStatus
	};
}

FacilityManager::Response FacilityManager::getCurrentRun (const FacilityManager::Query& query) {
	return {
		"getCurrentRun",
		json::array ({mSettings.getNumberOfRun() }),
		true
	};
}

FacilityManager::Response FacilityManager::getTriggerFrequency (const Query& query) const {
	double freq = -1;
	auto resposeStatus = false;
	if (isCtudcReadManager (mProcess) ) {
		auto ctudcManager = dynamic_cast<CtudcReadManager*> (mProcess.get() );
		freq = ctudcManager->getTriggerFrequency();
		resposeStatus = true;
	}
	return {
		"getTriggerFrequency",
		json::array ({freq}),
		resposeStatus
	};
}

FacilityManager::Response FacilityManager::getPackageFrequency (const FacilityManager::Query& query) const {
	double freq = -1;
	auto resposeStatus = false;
	if (isCtudcReadManager (mProcess) ) {
		auto ctudcManager = dynamic_cast<CtudcReadManager*> (mProcess.get() );
		freq = ctudcManager->getPackageFrequency();
		resposeStatus = true;
	}
	return {
		"getPackageFrequency",
		json::array ({freq}),
		resposeStatus
	};
}

FacilityManager::ProcessManagerPtr FacilityManager::createReadManager (const Query& query) const {
	const string type = query.input.at (0);
	if (type == "simple") {
		return make_unique<ReadManager> (mTdcModule,
		                                 mChannelConfig,
		                                 mSettings.getWriteDir(),
		                                 mSettings.getNumberOfRun(),
		                                 mSettings.getEventsPerFile() );
	} else if (type == "ctudc") {
		caen::CtudcReadManager::NetInfo netInfo{
			mSettings.getInfoPkgIp(), mSettings.getInfoPkgPort()
		};
		return make_unique<CtudcReadManager> (mTdcModule,
		                                      mChannelConfig,
		                                      mSettings.getWriteDir(),
		                                      mSettings.getNumberOfRun(),
		                                      mSettings.getEventsPerFile(),
		                                      netInfo);
	} else
	{ throw std::runtime_error ("Invalid query"); }
}

bool FacilityManager::isReadManager (const ProcessManagerPtr& processManager) const {
	if (!processManager) { return false; }
	const auto& ref = *processManager.get();
	auto& processType = typeid (ref);
	return 	processType == typeid (ReadManager);
}

bool FacilityManager::isFreqManager (const ProcessManagerPtr& processManager) const {
	if (!processManager) { return false; }
	const auto& ref = *processManager.get();
	auto& processType = typeid (ref);
	return 	processType == typeid (FrequencyManager);
}

bool FacilityManager::isCtudcReadManager (const ProcessManagerPtr& processManager) const {
	if (!processManager) { return false; }
	const auto& ref = *processManager.get();
	auto& processType = typeid (ref);
	return processType == typeid (CtudcReadManager);
}

json::array_t FacilityManager::convertFreq (const caen::TrekFrequency& freq) const {
	json::array_t jsonFreq;
	for (const auto& freqPair : freq) {
		const auto chamber = freqPair.first;
		const auto frequency = freqPair.second;

		jsonFreq.push_back ({
			{"chamber", chamber},
			{"frequency", frequency},
		});
	}
	return jsonFreq;
}

json::array_t FacilityManager::getProcessType (const ProcessManagerPtr& processManager) const {
	if (isReadManager (processManager) ) {
		return {"read"};
	} else if (isCtudcReadManager (processManager) ) {
		return {"ctudc"};
	} else if (isFreqManager (processManager) ) {
		return {"frequency"};
	} else {
		return {"null"};
	}
}

trekdata::Settings FacilityManager::createSettings (const Query& query) const {
	trekdata::Settings settings;
	const auto& input = query.input;
	settings.setTriggerMode ( input.at (0) );
	settings.setTriggerSubtraction ( input.at (1) );
	settings.setTdcMeta ( input.at (2) );
	settings.setWindowWidth ( input.at (3) );
	settings.setWindowOffset ( input.at (4) );
	uint16_t lsb = input.at (5);
	settings.setEdgeDetection ( static_cast<EdgeDetection> (lsb) );
	uint16_t edgeDetection = input.at (5);
	settings.setLsb ( static_cast<Lsb> (edgeDetection) );
	settings.setAlmostFull ( input.at (7) );
	settings.setControlRegister ( input.at (8) );
	settings.setDeadTime (input.at (9) );
	settings.setEventBLT (input.at (10) );
	return settings;
}

std::string FacilityManager::createWriteDir() const {
	ostringstream stream (mSettings.getWriteDir() );
	stream << "/data_set_" << setw (5) << setfill ('0')
	       << mSettings.getNumberOfRun();
	return stream.str();
}

void FacilityManager::stopReadCallback() {
	if (mStopReadCallback)
	{ mStopReadCallback (*this); }
}



nlohmann::json FacilitySettings::marshal() const {
	return {
		{"vme_address", getVmeAddress() },
		{"number_of_run", getNumberOfRun() },
		{"events_per_file", getEventsPerFile() },
		{"write_dir", getWriteDir() },
		{"info_pkg_ip", getInfoPkgIp() },
		{"info_pkg_port", getInfoPkgPort() }
	};
}

void FacilitySettings::unMarshal (const nlohmann::json& doc) {
	setVmeAddress (doc.at ("vme_address").get<int32_t>() );
	setNumberOfRun (doc.at ("number_of_run").get<uint64_t>() );
	setEventsPerFile (doc.at ("events_per_file").get<uintmax_t>() );
	setWriteDir (doc.at ("write_dir").get<string>() );
	setInfoPkgIp (doc.at ("info_pkg_ip").get<string>() );
	setInfoPkgPort (doc.at ("info_pkg_port") );
}

int32_t FacilitySettings::getVmeAddress() const {
	return mVmeAddress;
}

uint64_t FacilitySettings::getNumberOfRun() const {
	return mNumberOfRun;
}

uintmax_t FacilitySettings::getEventsPerFile() const {
	return mEventsPerFile;
}

const std::string& FacilitySettings::getWriteDir() const {
	return mWriteDir;
}

const std::string& FacilitySettings::getInfoPkgIp() const {
	return mInfoPkgIp;
}

uint16_t FacilitySettings::getInfoPkgPort() const {
	return mInfoPkgPort;
}

void FacilitySettings::setVmeAddress (int32_t vmeAddress) {
	mVmeAddress = vmeAddress;
}

void FacilitySettings::setNumberOfRun (uint64_t numberOfRun) {
	mNumberOfRun = numberOfRun;
}

void FacilitySettings::setEventsPerFile (uintmax_t eventsPerFile) {
	mEventsPerFile = eventsPerFile;
}

void FacilitySettings::setWriteDir (const std::string& dir) {
	mWriteDir = dir;
}

void FacilitySettings::setInfoPkgIp (const std::string& ip) {
	mInfoPkgIp = ip;
}

void FacilitySettings::setInfoPkgPort (uint16_t port) {
	mInfoPkgPort = port;
}
