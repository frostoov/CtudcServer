#include "appsettings.hpp"

#include "controller/tdccontroller.hpp"
#include "controller/processcontroller.hpp"
#include "controller/ftdcontroller.hpp"

#include "configparser/channelsconfigparser.hpp"

#include <trek/net/server.hpp>
#include <trek/common/stringbuilder.hpp>
#include <trek/common/applog.hpp>

#include <iostream>
#include <string>


using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::istreambuf_iterator;
using std::make_shared;
using std::chrono::system_clock;

using trek::StringBuilder;
using trek::Log;

using nlohmann::json;

void panic(const std::string& message) {
    Log::instance() << message << endl;
    cin.ignore();
    std::exit(0);
}

int main() {
    std::ios_base::sync_with_stdio(false);
    Log::init("CtudcServer.log");
    Log::instance().setStdOut(true);

    AppSettings appSettings;
    try {
        appSettings.load("CtudcServer.conf");
    } catch(const std::exception& e) {
        panic(StringBuilder() << "Failed parse CtudcServer.conf: " << e.what());
    }

    ChannelsConfigParser channelParser;
    try {
        channelParser.load("channels.conf");
    } catch(const std::exception& e) {
        panic(StringBuilder() << "Failed parse channels.conf: " << e.what());
    }

    auto tdcController  = make_shared<TdcController>(0xEE00);
    auto ftdController  = make_shared<FtdController>(0x28);
    auto procController = make_shared<ProcessController>(tdcController->getModule(),
                                                         channelParser.getConfig(),
                                                         appSettings.processSettings());
    procController->onStopRead() = [&](const ProcessController& procController) {
        appSettings.setProcessSettings(procController.getSettings());
        appSettings.save("CtudcServer.conf");
    };

    trek::net::Server server({tdcController, ftdController, procController},
                             appSettings.ipAddress(),
                             appSettings.port());
    server.onStart() = [](const auto&) {
        Log::instance() << system_clock::now() << " Server start" << endl;
    };
    server.onStop() = [](const auto&) {
        Log::instance() << system_clock::now() << " Server start" << endl;
    };
    server.onSessionStart() = [](const trek::net::Session& session) {
        Log::instance() << system_clock::now() << " Connected: " << session.getRemoteAddress() << endl;
    };
    server.onSessionClose() = [](const trek::net::Session& session) {
        Log::instance() << system_clock::now() << " Disconnected: " << session.getRemoteAddress() << endl;
    };
    server.onRecv() = [](const auto& session, const auto& message) {
        Log::instance() << system_clock::now() << " Recv " << session.getRemoteAddress() << ": " << message << endl;
    };
    server.onSend() = [](const auto& session, const auto& message) {
        Log::instance() << system_clock::now() << " Send " << session.getRemoteAddress() << ": " << message << endl;
    };

    server.run();
    string command;
    while(true) {
        std::getline(cin, command);
        if(command == "start")
            server.run();
        else if(command == "stop")
            server.stop();
        else if(command == "exit") {
            server.stop();
            std::exit(0);
        }
    }
    return 0;
}
