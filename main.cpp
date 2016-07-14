#include "appsettings.hpp"

#include "controller/emisscontr.hpp"
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
using std::exception;
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

    string confPath = std::getenv("HOME");
    if(!confPath.empty()) {
        confPath += "/.config/ctudc/";
    }
    AppSettings appSettings;
    try {
        appSettings.load(confPath + "CtudcServer.conf");
    } catch(const std::exception& e) {
        fatal(StringBuilder() << "Failed parse CtudcServer.conf: " << e.what());
    }

    ChannelsConfigParser channelParser;
    try {
        channelParser.load(confPath + "channels.conf");
    } catch(std::exception& e) {
        fatal(StringBuilder() << "Failed parse channels.conf: " << e.what());
    }

    auto caentdc = make_shared<CaenV2718>(0xEE00);
    //auto emisstdc = make_shared<EmissTdc>();
    auto ftd = make_shared<ftdi::Module>(0x28);
    auto vlt = make_shared<Amplifier>();
    try {
        ftd->open("C232HM-EDHSL-0");
        ftd->initialize({ftdi::I2C_CLOCK_STANDARD_MODE, 1, 0});
    } catch(std::exception& e) {
        std::cerr << "Failed open ftd: " << e.what() << std::endl;
    }
    try {
        vlt->open("/dev/my_uart");
    } catch(exception& e) {
        std::cerr << "Failed open voltage: " << e.what() << std::endl;
    }
    try {
        caentdc->open();
    } catch(exception& e) {
        std::cerr << "Failed open caentdc: " << e.what() << std::endl;
    }

    auto tdcController  = make_shared<Caen2718Contr>("tdc", caentdc);
    //auto emissController= make_shared<EmissContr>("emiss", emisstdc);
    auto vltController  = make_shared<VoltageContr>("vlt", vlt, ftd, appSettings.voltConfig);
    auto expoController = make_shared<ExpoContr>("expo", caentdc, appSettings.expoConfig, channelParser.getConfig());
    expoController->onNewRun() = [&](unsigned nRun) {
        appSettings.expoConfig.nRun = nRun;
        appSettings.save(confPath + "CtudcServer.conf");
    };
    

    trek::net::Server server({tdcController, expoController, vltController}, {
        appSettings.ip,
        appSettings.port,
        appSettings.multicastIp,
        appSettings.multicastPort,
    });
    server.onStart() = [](const auto&) {
        std::cout << system_clock::now() << " Server start" << endl;
    };
    server.onStop() = [](const auto&) {
        std::cout << system_clock::now() << " Server stop" << endl;
    };
    server.onSessionStart() = [](const trek::net::Session & session) {
        std::cout << system_clock::now() << " Connected: " << session.remoteAddress() << endl;
    };
    server.onSessionClose() = [](const trek::net::Session & session) {
        std::cout << system_clock::now() << " Disconnected: " << session.remoteAddress() << endl;
    };
    server.onRecv() = [](const auto & session, const auto & message) {
        std::cout << system_clock::now() << " Recv " << session.remoteAddress() << ": " << message << endl;
    };
    server.onSend() = [](const auto & session, const auto & message) {
        std::cout << system_clock::now() << " Send " << session.remoteAddress() << ": " << message << endl;
    };

    std::atomic_bool runnig(true);
    auto future = std::async(std::launch::async, [&] {
        while(runnig.load()) {
            try {
                server.run();
            } catch(exception& e) {
                std::cout << system_clock::now() << " Server failed: " << e.what() << std::endl;
                server.stop();
            }
        }
    });
    string command;
    while(true) {
        std::getline(cin, command);
        if(command == "exit") {
            if(future.valid()) {
                runnig.store(false);
                server.stop();
                future.get();
            }
            break;
        }
    }
    return 0;
}
