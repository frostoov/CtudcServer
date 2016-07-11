#pragma once

#include "configparser/appconfigparser.hpp"
#include "exposition/exposition.hpp"
#include "controller/voltagecontroller.hpp"

struct AppSettings {
    std::string ip;
    uint16_t    port;
    std::string multicastIp;
    uint16_t    multicastPort;
    Exposition::Settings expoConfig;
    VoltageContr::Config voltConfig;

    void load(const std::string& fileName);
    void save(const std::string& fileName);

    nlohmann::json marshal() const;
    void unMarshal(const nlohmann::json& doc);
};
