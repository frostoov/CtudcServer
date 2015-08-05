#include "net/server.hpp"
#include "configparser/channelsconfigparser.hpp"
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
	std::ios_base::sync_with_stdio(false);
	string address;
	uint16_t port;
	ifstream stream;
	stream.exceptions(stream.badbit | stream.failbit);
	try {
		stream.open("psychoServer.conf");
	} catch(const std::exception& e) {
		cout << "Failed open psychoServer.conf: " << e.what() << endl;
		cin.ignore();
		std::exit(0);
	}
	try {
		json config = json::parse({istreambuf_iterator<char>(stream), istreambuf_iterator<char>()});
		address = config.at("address").get<string>();
		port = config.at("port").get<uint16_t>();
	} catch(const std::exception& e) {
		cout << "Failed open parse psychoServer.conf: " << e.what() << endl;
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

	auto deviceManager = make_shared<DeviceManager>(0xEE00, channelParser.getConfig());
	Server server(deviceManager, address, port);
	server.start();
	while(true)
		cin.ignore();
	return 0;
}


