#pragma once

#include "configparser/appconfigparser.hpp"
#include "controller/voltagecontroller.hpp"
#include "controller/expocontroller.hpp"

struct AppSettings {
    std::string ip;
    uint16_t    port;
    std::string multicastIp;
    uint16_t    multicastPort;
    ExpoContr::Settings expoConfig;
    VoltageContr::Config voltConfig;

    void load(const std::string& fileName);
    void save(const std::string& fileName);

    nlohmann::json marshal() const;
    void unMarshal(const nlohmann::json& doc);
};
