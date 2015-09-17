#ifndef APPSETTINGS_HPP
#define APPSETTINGS_HPP

#include "configparser/appconfigparser.hpp"
#include "controller/processcontroller.hpp"

namespace {
using ProcSettings = ProcessController::Settings;
}

class AppSettings {
public:
    AppSettings() = default;
    void load(const std::string& fileName);
    void save(const std::string& fileName);

    nlohmann::json marshal() const;
    void unMarshal(const nlohmann::json& doc);

    const std::string& ipAddress() const;
    uint16_t port() const;
    const ProcSettings& processSettings() const;

    void setIpAddress(const std::string& address);
    void setPort(uint16_t port);
    void setProcessSettings(const ProcSettings& settings);

private:
    std::string  mIpAddress;
    uint16_t     mPort;
    ProcSettings mProcSettings;
};

#endif // APPSETTINGS_HPP
