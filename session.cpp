#include <json.hpp>
#include <iostream>
#include "session.hpp"

using std::string;
using std::cout;
using std::endl;
using std::cerr;

using nlohmann::json;

using caen::Module;
using tdcdata::Lsb;
using tdcdata::EdgeDetection;

Session::Session(ModulePtr device, Socket&& socket, DestroyCallback callback)
	: mDevice(device),
	  mSocket(std::move(socket)),
	  mCallback(callback) {
	mProcedures = createProcedures();
}

Session::~Session() {
	cout << "~Session" << endl;
}

string Session::createAnswer(const Response& response) {
	return json {
		{"output", response.output},
		{"status", response.status},
	} .dump();
}

Session::Procedures Session::createProcedures() {
	return {
		{"init",                   [&](const Query & query)	{ return this->init(query);}},
		{"close",                  [&](const Query & query)	{ return this->close(query);}},
		{"isInit",                 [&](const Query & query) { return this->isInit(query);}},
		{"getSettings",            [&](const Query & query)	{ return this->getSettings(query);}},
		{"setSettings",            [&](const Query & query)	{ return this->setSettings(query);}},
		{"updateSettings",         [&](const Query & query) { return this->updateSettings(query);}},
		{"getLog",                 [&](const Query & query)	{ return this->getLog(query);}},
		{"setLog",                 [&](const Query & query) { return this->setLog(query);}},
		{"setTriggerMode",	       [&](const Query & query)	{ return this->setTriggerMode(query);}},
		{"setTriggerSubtraction",  [&](const Query & query)	{ return this->setTriggerSubtraction(query);}},
		{"setTdcMeta",             [&](const Query & query)	{ return this->setTdcMeta(query);}},
		{"setWindowWidth",         [&](const Query & query)	{ return this->setWindowWidth(query);}},
		{"setWindowOffset",        [&](const Query & query)	{ return this->setWindowOffset(query);}},
		{"setEdgeDetection",       [&](const Query & query)	{ return this->setEdgeDetection(query);}},
		{"setLsb",                 [&](const Query & query)	{ return this->setLsb(query);}},
		{"setAlmostFull",          [&](const Query & query)	{ return this->setAlmostFull(query);}},
		{"setControl",             [&](const Query & query)	{ return this->setControl(query);}},
		{"setDeadTime",            [&](const Query & query)	{ return this->setDeadTime(query);}},
		{"setEventBLT",            [&](const Query & query)	{ return this->setEventBLT(query);}},
	};
}

void Session::workerLoop() {
	setLoop(true);
	std::array<char, 65536> buffer;
	std::string answer;
	while(isActive()) {
		try {
			auto bytesRead = mSocket.receive(boost::asio::buffer(buffer));
			try {
				std::string message(buffer.data(), bytesRead);
				cout << "Received query:\n" << message << endl;
				answer = handleMessage(json::parse(message));
			} catch(const std::exception& e) {
				answer = "Invalid request";
			}
			mSocket.send(boost::asio::buffer(answer));
		} catch(const std::exception& e) {
			cerr << e.what() << endl;
			setActive(false);
		}
	}
	cout << "Disconnected: " << mSocket.remote_endpoint().address().to_string() << endl;
	setLoop(false);
	mCallback(shared_from_this());
}

string Session::handleMessage(const json& message) {
	Query query = {
		message.at("procedure"),
		message.at("input"),
	};
	const auto& procedure = mProcedures.at(query.procedure);
	auto respone = procedure(query);
	return createAnswer(respone);
}

Session::Response Session::isInit(const Query&) {
	return {
		json::array({mDevice->isInit()}),
		true,
	};
}

Session::Response Session::getSettings(const Query&) {
	const auto& settings = mDevice->getSettings();
	return {{
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

Session::Response Session::updateSettings(const Query&) {
	return {
		json::array(),
		mDevice->updateSettings(),
	};
}

Session::Response Session::getLog(const Query&) {
	Response response;
	while(mDevice->hasMessages())
		response.output.push_back(mDevice->popMessage().first);
	response.status = true;
	return response;
}

Session::Response Session::init(const Query& query) {
	return {
		json::array(),
		mDevice->initialize(),
	};
}

Session::Response Session::close(const Query&) {
	return {
		json::array(),
		mDevice->close(),
	};
}

Session::Response Session::setSettings(const Query& query) {
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
		json::array(),
		mDevice->setSettings(settings),
	};
}

Session::Response Session::setLog(const Query& query) {
	mDevice->setLog(query.input.at(0));
	return {
		json::array(),
		true,
	};
}

Session::Response Session::setTriggerMode(const Query& query) {
	return {
		json::array(),
		mDevice->setTriggerMode(query.input.front()),
	};
}

Session::Response Session::setTriggerSubtraction(const Query& query) {
	return {
		json::array(),
		mDevice->setTriggerSubtraction(query.input.front()),
	};
}

Session::Response Session::setTdcMeta(const Session::Query& query) {
	return {
		json::array(),
		mDevice->setTdcMeta(query.input.front()),
	};
}

Session::Response Session::setWindowWidth(const Session::Query& query) {
	return {
		json::array(),
		mDevice->setWindowWidth(query.input.front()),
	};
}

Session::Response Session::setWindowOffset(const Session::Query& query) {
	return {
		json::array(),
		mDevice->setWindowOffset(query.input.front()),
	};
}

Session::Response Session::setEdgeDetection(const Session::Query& query) {
	auto edgeDetection = static_cast<uint16_t>(query.input.front());
	return {
		json::array(),
		mDevice->setEdgeDetection(static_cast<EdgeDetection>(edgeDetection)),
	};
}

Session::Response Session::setLsb(const Session::Query& query) {
	auto lsb = static_cast<uint16_t>(query.input.front());
	return {
		json::array(),
		mDevice->setLsb(static_cast<Lsb>(lsb)),
	};
}

Session::Response Session::setAlmostFull(const Session::Query& query) {
	return {
		json::array(),
		mDevice->setAlmostFull(query.input.front()),
	};
}

Session::Response Session::setControl(const Session::Query& query) {
	return {
		json::array(),
		mDevice->setControl(query.input.front()),
	};
}

Session::Response Session::setDeadTime(const Session::Query& query) {
	return {
		json::array(),
		mDevice->setDeadTime(query.input.front()),
	};
}

Session::Response Session::setEventBLT(const Session::Query& query) {
	return {
		json::array(),
		mDevice->setEventBLT(query.input.front()),
	};
}
