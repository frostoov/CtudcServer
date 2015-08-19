#include <iostream>
#include <string>

#include "net/server.hpp"
#include "appsettings.hpp"
#include "configparser/channelsconfigparser.hpp"


using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::istreambuf_iterator;
using std::make_shared;

using nlohmann::json;

int main() {
	std::ios_base::sync_with_stdio(false);

	AppSettings appSettings;
	try {
		appSettings.load("CtudcServer.conf");
	} catch(const std::exception& e) {
		cout << "Failed parse CtudcServer.conf: " << e.what() << endl;
		cin.ignore();
		std::exit(0);
	}

	ChannelsConfigParser channelParser;
	try {
		channelParser.load("channels.conf");
	} catch(const std::exception& e) {
		cout << "Failed parse channels.conf: " << e.what() << endl;
		cin.ignore();
		std::exit(0);
	}

	auto deviceManager = make_shared<FacilityManager> (0xEE00,
													   channelParser.getConfig(),
													   appSettings.getFacilitySettings());
	deviceManager->setStopReadCallback([&appSettings](const FacilityManager & manager) {
		appSettings.setFacilitySettings(manager.getSettings());
		appSettings.save("CtudcServer.conf");
	});
	Server server(deviceManager, appSettings.getIpAddress(), appSettings.getPort());
	server.start();

	string command;
	while(true) {
		std::getline(cin, command);
		if(command == "exit") {
			server.stop();
			cin.ignore();
			exit(0);
		}
	}
	return 0;
}


