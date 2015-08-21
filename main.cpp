#include <iostream>
#include <string>


#include "controller/tdccontroller.hpp"
#include "controller/ftdcontroller.hpp"
#include "controller/processcontroller.hpp"

#include "net/server.hpp"
#include "appsettings.hpp"
#include "configparser/channelsconfigparser.hpp"

#include "makestring.hpp"


using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::istreambuf_iterator;
using std::make_shared;

using nlohmann::json;

void panic(const std::string& message) {
    cout << message << endl;
    cin.ignore();
    std::exit(0);
}

int main() {
    std::ios_base::sync_with_stdio(false);

    AppSettings appSettings;
    try {
        appSettings.load("CtudcServer.conf");
    } catch(const std::exception& e) {
        panic(MakeString() << "Failed parse CtudcServer.conf: " << e.what());
    }

    ChannelsConfigParser channelParser;
    try {
        channelParser.load("channels.conf");
    } catch(const std::exception& e) {
        panic(MakeString() << "Failed parse channels.conf: " << e.what());
    }

    auto tdcController  = make_shared<ctudc::TdcController>(0xEE00);
    auto ftdController  = make_shared<ctudc::FtdController>(0x28);
    auto procController = make_shared<ctudc::ProcessController>(tdcController->getModule(),
                                                                channelParser.getConfig(),
                                                                appSettings.getProcessSettings());
    procController->connectStopRead([&](const ctudc::ProcessController& procController) {
        appSettings.setProcessSettings(procController.getSettings());
        appSettings.save("CtudcServer.conf");
    });
    std::unordered_map<string, std::shared_ptr<Controller>> controllers {
        {tdcController->getName(), tdcController},
        {ftdController->getName(), ftdController},
        {procController->getName(), procController},
    };

    Server server(controllers, appSettings.getIpAddress(), appSettings.getPort());
    server.start();
    string command;
    while(true) {
        std::getline(cin, command);
        if(command == "start")
            server.start(); else if(command == "stop")
            server.stop(); else if(command == "exit") {
            server.stop();
            std::exit(0);
        }
    }
    return 0;
}


