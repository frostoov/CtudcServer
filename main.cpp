#include <iostream>
#include <string>
#include <array>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <json.hpp>
#include <unordered_map>

#include "caenTDC/tdcmodule.hpp"

using namespace boost::asio;
using boost::system::error_code;
using boost::asio::ip::tcp;

using caen::Module;
using nlohmann::json;

std::string getLog(Module& module) {
	std::string Log;
	while(module.hasMessages()) {
		Log += module.popMessage().first + '\n';
	}
	return Log;
}

void init(Module& module) {
	module.initialize();
}

void close(Module& module) {
	module.close();
}

void setSetting(Module& module, const json& input) {
	module.setWindowWidth(input.at("WindowWidth"));
	module.setWindowOffset(input.at("WindowOffset"));
}

void updateSettings(Module& module) {
	module.updateSettings();
}

json getSettings(Module& module) {
	const auto& settings = module.getSettings();
	return json {
		{"TriggerMode", settings.getTriggerMode()},
		{"TriggerSubtraction", settings.getTriggerSubtraction()},
		{"TdcMeta", settings.getTdcMeta()},
		{"WindowWidth", settings.getWindowWidth()},
		{"WindowOffset", settings.getWindowOffset()},
		{"EdgeDetection", static_cast<uint16_t>(settings.getEdgeDetection())},
		{"Lsb", static_cast<uint16_t>(settings.getLsb())},
		{"AlmostFull", settings.getAlmostFull()},
		{"Control", settings.getControl()},
		{"Status", settings.getStatus()},
		{"DeadTime", settings.getDeadTime()},
		{"EventBLT", settings.getEventBLT()}
	};
}

std::string handleMessage(Module& module, const json& call) {
	const auto& procedureName = call.at("procedure");
	const auto& id = call.at("id");
	json output {
		{"procedure", procedureName},
		{"id", id}
	};
	if(procedureName == "init") {
		init(module);
	} else if(procedureName == "close") {
		close(module);
	} else if(procedureName == "getSettings") {
		output["output"] = getSettings(module);
	} else if(procedureName == "setSettigns") {
		setSetting(module, call.at("input"));
	} else if (procedureName == "updateSettings") {
		updateSettings(module);
	}
	output["status"] = getLog(module);
	return output.dump();
}

int main(){
	Module module(0xEE00);
	module.setLog(true);
	try {
		boost::asio::io_service io_service;
		tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 8081));

		while(true) {
			tcp::socket socket(io_service);
			acceptor.accept(socket);
			const std::string addr(socket.remote_endpoint().address().to_string());
			std::cout << "accept: " << addr << std::endl;

			std::array<char, 1024*1024> buffer;
			try {
				while(true) {
					auto bytesRead = socket.read_some(boost::asio::buffer(buffer));
					std::string message(buffer.data(), bytesRead);
					try {
						auto answer = handleMessage(module, json::parse(message));
						socket.write_some(boost::asio::buffer(answer));
					} catch(...) {
						socket.write_some(boost::asio::buffer("Invalid command"));
					}
				}
			} catch(const std::exception& e) {
				std::cout << "Lost connection: " << addr << ": " << e.what() << std::endl;
			}
		}
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}


