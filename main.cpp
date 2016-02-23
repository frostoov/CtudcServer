#include "appsettings.hpp"

#include "controller/tdccontroller.hpp"
#include "controller/expocontroller.hpp"
#include "controller/voltagecontroller.hpp"

#include "configparser/channelsconfigparser.hpp"

#include <trek/net/server.hpp>
#include <trek/common/timeprint.hpp>
#include <trek/common/stringbuilder.hpp>

#include <iostream>
#include <string>
#include <future>


using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::istreambuf_iterator;
using std::make_shared;
using std::chrono::system_clock;

using trek::StringBuilder;

using nlohmann::json;

void fatal(const string& msg) {
	std::cerr << msg << endl;
	std::exit(EXIT_FAILURE);
}

int main() {
	std::ios_base::sync_with_stdio(false);

	AppSettings appSettings;
	try {
		appSettings.load("CtudcServer.conf");
	} catch(const std::exception& e) {
		fatal(StringBuilder() << "Failed parse CtudcServer.conf: " << e.what());
	}

	ChannelsConfigParser channelParser;
	try {
		channelParser.load("channels.conf");
	} catch(const std::exception& e) {
		fatal(StringBuilder() << "Failed parse channels.conf: " << e.what());
	}

	auto tdc = make_shared<CaenV2718>(0xEE00);
	auto vlt = make_shared<Amplifier>();
	vlt->setTimeout(5000);

	auto tdcController  = make_shared<TdcController>("tdc", tdc);
	auto vltController  = make_shared<VoltageController>("vlt", vlt, appSettings.voltConfig);
	auto expoController = make_shared<ExpoController>(
	                          "process",
	                          tdc,
	                          appSettings.expoSettings,
	                          channelParser.getConfig()
	                      );
	expoController->onNewRun() = [&](unsigned nRun) {
		appSettings.expoSettings.nRun = nRun;
		appSettings.save("CtudcServer.conf");
	};


	trek::net::Server server(
	{tdcController, expoController, vltController},
	appSettings.ip,
	appSettings.port
	);
	server.onStart() = [](const auto&) {
		std::cout << system_clock::now() << " Server start" << endl;
	};
	server.onStop() = [](const auto&) {
		std::cout << system_clock::now() << " Server stop" << endl;
	};
	server.onSessionStart() = [](const trek::net::Session & session) {
		std::cout << system_clock::now() << " Connected: " << session.getRemoteAddress() << endl;
	};
	server.onSessionClose() = [](const trek::net::Session & session) {
		std::cout << system_clock::now() << " Disconnected: " << session.getRemoteAddress() << endl;
	};
	server.onRecv() = [](const auto & session, const auto & message) {
		std::cout << system_clock::now() << " Recv " << session.getRemoteAddress() << ": " << message << endl;
	};
	server.onSend() = [](const auto & session, const auto & message) {
		std::cout << system_clock::now() << " Send " << session.getRemoteAddress() << ": " << message << endl;
	};

	auto future = std::async(std::launch::async, [&] {server.run(); } );
	string command;
	while(true) {
		std::getline(cin, command);
		if(command == "run" && !future.valid())
			future = std::async(std::launch::async, [&] {server.run(); } );
		else if(command == "stop" && future.valid()) {
			server.stop();
			future.get();
		} else if(command == "exit") {
			if(future.valid()) {
				server.stop();
				future.get();
			}
			break;
		}
	}
	return 0;
}
