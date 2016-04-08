#pragma once

#include "voltage/amplifier.hpp"
#include "ftd/ftdmodule.hpp"

#include <trek/net/controller.hpp>
#include <json.hpp>

class VoltageController : public trek::net::Controller {
    using Module = Amplifier;
    using ModulePtr = std::shared_ptr<Module>;
    using FtdPtr = std::shared_ptr<ftdi::Module>;
public:
    struct Config {
        int signal;
        int drift;

        nlohmann::json marshal() const;
        void unMarhsal(const nlohmann::json& json);
    };
public:
    VoltageController(const std::string& name, const ModulePtr& module, const FtdPtr& ftd, const Config& config);
protected:
    Methods createMethods();

    void open(const trek::net::Request& request, const SendCallback& send);
    void close(const trek::net::Request& request, const SendCallback& send);
    void isOpen(const trek::net::Request& request, const SendCallback& send);

    void stat(const trek::net::Request& request, const SendCallback& send);
    void turnOn(const trek::net::Request& request, const SendCallback& send);
    void turnOff(const trek::net::Request& request, const SendCallback& send);
    void setVoltage(const trek::net::Request& request, const SendCallback& send);
    void setSpeedUp(const trek::net::Request& request, const SendCallback& send);
    void setSpeedDn(const trek::net::Request& request, const SendCallback& send);
    void speedUp(const trek::net::Request& request, const SendCallback& send);
    void speedDn(const trek::net::Request& request, const SendCallback& send);
    void voltage(const trek::net::Request& request, const SendCallback& send);
    void amperage(const trek::net::Request& request, const SendCallback& send);
    int getCell(const std::string& name);
private:
    ModulePtr mDevice;
    FtdPtr    mFtd;
    Config    mConfig;
};
