#include "appsettings.hpp"

using std::string;

void AppSettings::load(const std::string& fileName) {
    AppConfigParser parser;
    parser.load(fileName);

    unMarshal(parser.config());
}

void AppSettings::save(const std::string& fileName) {
    AppConfigParser parser(marshal());
    parser.save(fileName);
}

nlohmann::json AppSettings::marshal() const {
    return {
        {"address", ip },
        {"port", port },
        {"multicast_address", multicastIp},
        {"multicast_port", multicastPort},
        {"expo", expoConfig.marshal()},
        {"voltage", voltConfig.marshal()},
    };
}

void AppSettings::unMarshal(const nlohmann::json& doc) {
    ip   = doc.at("address").get<string>();
    port = doc.at("port");
    multicastIp = doc.at("multicast_address").get<string>();
    multicastPort = doc.at("multicast_port");
    expoConfig.unMarshal(doc.at("expo"));
    voltConfig.unMarhsal(doc.at("voltage"));
}
