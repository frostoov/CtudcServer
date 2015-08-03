#include "devicemanager.hpp"
#include "tdcdata/tdcsettings.hpp"

using nlohmann::json;
using tdcdata::EdgeDetection;
using tdcdata::Lsb;

using std::string;

DeviceManager::DeviceManager(int32_t vmeAddress)
	: mDevice(vmeAddress),
	  mProcedures(createProcedures()) { }

DeviceManager::Procedures DeviceManager::createProcedures() {
	return {
		{"init",                   [&](const Query & query) { return this->init(query);}},
		{"close",                  [&](const Query & query) { return this->close(query);}},
		{"isInit",                 [&](const Query & query) { return this->isInit(query);}},
		{"getSettings",            [&](const Query & query) { return this->getSettings(query);}},
		{"setSettings",            [&](const Query & query) { return this->setSettings(query);}},
		{"updateSettings",         [&](const Query & query) { return this->updateSettings(query);}},
		{"getLog",                 [&](const Query & query) { return this->getLog(query);}},
		{"setLog",                 [&](const Query & query) { return this->setLog(query);}},
		{"setTriggerMode",	       [&](const Query & query) { return this->setTriggerMode(query);}},
		{"setTriggerSubtraction",  [&](const Query & query) { return this->setTriggerSubtraction(query);}},
		{"setTdcMeta",             [&](const Query & query) { return this->setTdcMeta(query);}},
		{"setWindowWidth",         [&](const Query & query) { return this->setWindowWidth(query);}},
		{"setWindowOffset",        [&](const Query & query) { return this->setWindowOffset(query);}},
		{"setEdgeDetection",       [&](const Query & query) { return this->setEdgeDetection(query);}},
		{"setLsb",                 [&](const Query & query) { return this->setLsb(query);}},
		{"setAlmostFull",          [&](const Query & query) { return this->setAlmostFull(query);}},
		{"setControl",             [&](const Query & query) { return this->setControl(query);}},
		{"setDeadTime",            [&](const Query & query) { return this->setDeadTime(query);}},
		{"setEventBLT",            [&](const Query & query) { return this->setEventBLT(query);}},
	};
}

string DeviceManager::handleQuery(const string& rawQuery) {
	auto query = convertQuery(rawQuery);
	auto procedure = getProcedure(query);
	auto response = procedure(query);
	return convertResponse(response);
}

DeviceManager::Query DeviceManager::convertQuery(const string& rawQuery) {
	auto query = json::parse(rawQuery);
	return {
		query.at("procedure"),
		query.at("input"),
	};
}

string DeviceManager::convertResponse(const DeviceManager::Response& response) {
	return json{
		{"procedure", response.procedure},
		{"output",    response.output},
		{"status",    response.status},
	}.dump();
}

DeviceManager::Procedure DeviceManager::getProcedure(const DeviceManager::Query& query) const {
	return mProcedures.at(query.procedure);
}

DeviceManager::Response DeviceManager::init(const Query& query) {
	return {
		"init",
		json::array(),
		mDevice.initialize(),
	};
}

DeviceManager::Response DeviceManager::close(const Query& query) {
	return {
		"close",
		json::array(),
		mDevice.close(),
	};
}

DeviceManager::Response DeviceManager::isInit(const Query& query) {
	return {
		"isInit",
		json::array({mDevice.isInit()}),
		true,
	};
}

DeviceManager::Response DeviceManager::setLog(const Query& query) {
	mDevice.setLog(query.input.at(0));
	return {
		"setLog",
		json::array(),
		true,
	};
}

DeviceManager::Response DeviceManager::getLog(const Query& query) {
	json::array_t log;
	while(mDevice.hasMessages())
		log.push_back(mDevice.popMessage().first);
	return {
		"getLog",
		log,
		true,
	};
}

DeviceManager::Response DeviceManager::getSettings(const Query& query) {
	const auto& settings = mDevice.getSettings();
	return {
		"getSettings",
		{

			settings.getTriggerMode(),
			settings.getTriggerSubtraction(),
			settings.getTdcMeta(),
			settings.getWindowWidth(),
			settings.getWindowOffset(),
			static_cast<uint16_t>(settings.getEdgeDetection()),
			static_cast<uint16_t>(settings.getLsb()),
			settings.getAlmostFull(),
			settings.getControl(),
			settings.getStatus(),
			settings.getDeadTime(),
			settings.getEventBLT()
		},
		true
	};
}

DeviceManager::Response DeviceManager::updateSettings(const Query& query) {
	return {
		"updateSettings",
		json::array(),
		mDevice.updateSettings(),
	};
}

DeviceManager::Response DeviceManager::setSettings(const Query& query) {
	tdcdata::Settings settings;
	const auto& input = query.input;
	settings.setTriggerMode( input.at(0) );
	settings.setTriggerSubtraction( input.at(1) );
	settings.setTdcMeta( input.at(2) );
	settings.setWindowWidth( input.at(3) );
	settings.setWindowOffset( input.at(4) );
	uint16_t lsb = input.at(5);
	settings.setEdgeDetection( static_cast<EdgeDetection>(lsb) );
	uint16_t edgeDetection = input.at(5);
	settings.setLsb( static_cast<Lsb>(edgeDetection) );
	settings.setAlmostFull( input.at(7) );
	settings.setControlRegister( input.at(8) );
	settings.setDeadTime(input.at(9));
	settings.setEventBLT(input.at(10));
	return {
		"setSettings",
		json::array(),
		mDevice.setSettings(settings),
	};
}

DeviceManager::Response DeviceManager::setTriggerMode(const Query& query) {
	return {
		"setTriggerMode",
		json::array(),
		mDevice.setTriggerMode(query.input.front()),
	};
}

DeviceManager::Response DeviceManager::setTriggerSubtraction(const Query& query) {
	return {
		"setTriggerSubtraction",
		json::array(),
		mDevice.setTriggerSubtraction(query.input.front()),
	};
}

DeviceManager::Response DeviceManager::setTdcMeta(const Query& query) {
	return {
		"setTdcMeta",
		json::array(),
		mDevice.setTdcMeta(query.input.front()),
	};
}

DeviceManager::Response DeviceManager::setWindowWidth(const Query& query) {
	return {
		"setWindowWidth",
		json::array(),
		mDevice.setWindowWidth(query.input.front()),
	};
}

DeviceManager::Response DeviceManager::setWindowOffset(const Query& query) {
	return {
		"setWindowOffset",
		json::array(),
		mDevice.setWindowOffset(query.input.front()),
	};
}

DeviceManager::Response DeviceManager::setEdgeDetection(const Query& query) {
	auto edgeDetection = static_cast<uint16_t>(query.input.front());
	return {
		"setEdgeDetection",
		json::array(),
		mDevice.setEdgeDetection(static_cast<EdgeDetection>(edgeDetection)),
	};
}

DeviceManager::Response DeviceManager::setLsb(const Query& query) {
	auto lsb = static_cast<uint16_t>(query.input.front());
	return {
		"setLsb",
		json::array(),
		mDevice.setLsb(static_cast<Lsb>(lsb)),
	};
}

DeviceManager::Response DeviceManager::setAlmostFull(const Query& query) {
	return {
		"setAlmostFull",
		json::array(),
		mDevice.setAlmostFull(query.input.front()),
	};
}

DeviceManager::Response DeviceManager::setControl(const Query& query) {
	return {
		"setControl",
		json::array(),
		mDevice.setControl(query.input.front()),
	};
}

DeviceManager::Response DeviceManager::setDeadTime(const Query& query) {
	return {
		"setDeadTime",
		json::array(),
		mDevice.setDeadTime(query.input.front()),
	};
}

DeviceManager::Response DeviceManager::setEventBLT(const Query& query) {
	return {
		"setEventBLT",
		json::array(),
		mDevice.setEventBLT(query.input.front()),
	};
}

