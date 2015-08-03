#include "server.hpp"
#include <iostream>
#include <fstream>
#include <string>

using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::ifstream;
using std::istreambuf_iterator;
using std::make_shared;

using nlohmann::json;

int main() {
	string address;
	uint16_t port;
	ifstream stream;
	stream.exceptions(stream.badbit | stream.failbit);
	try {
		stream.open("psychoServer.conf");
	} catch(const std::exception& e) {
		cout << "Failed open psychoServer.conf: " << e.what() << endl;
		std::exit(0);
	}
	try {
		json config = json::parse({istreambuf_iterator<char>(stream), istreambuf_iterator<char>()});
		address = config.at("address").get<string>();
		port = config.at("port").get<uint16_t>();
	} catch(const std::exception& e) {
		cout << "Failed open parse psychoServer.conf: " << e.what() << endl;
		std::exit(0);
	}

	auto deviceManager = make_shared<DeviceManager>(0xEE00);
	Server server(deviceManager, address, port);
	std::string command;
	while(true) {
		std::getline(cin,  command);
		if(command == "start") {
			server.start();
		} else if(command == "stop") {
			server.stop();
		}
	}
	return 0;
}


