#pragma once

#include "voltage/amplifier.hpp"
#include "ftd/ftdmodule.hpp"

#include <trek/net/controller.hpp>
#include <nlohmann/json.hpp>

#include <future>
#include <memory>

class VoltageContr : public trek::net::Controller {
    using Module = Amplifier;
    using ModulePtr = std::shared_ptr<Module>;
    using FtdPtr = std::shared_ptr<ftdi::Module>;
    class Arduino;
public:
    struct Config {
        int signal;
        int drift;

        nlohmann::json marshal() const;
        void unMarhsal(const nlohmann::json& json);
    };
public:
    VoltageContr(const std::string& name, const ModulePtr& module, const FtdPtr& ftd, const Config& config);
    ~VoltageContr();
protected:
    std::future<void> launchMonitoring();
    Methods createMethods();

    trek::net::Response open(const trek::net::Request& request);
    trek::net::Response close(const trek::net::Request& request);
    trek::net::Response isOpen(const trek::net::Request& request);

    trek::net::Response stat(const trek::net::Request& request);
    trek::net::Response turnOn(const trek::net::Request& request);
    trek::net::Response turnOff(const trek::net::Request& request);
    trek::net::Response setVoltage(const trek::net::Request& request);
    trek::net::Response setSpeedUp(const trek::net::Request& request);
    trek::net::Response setSpeedDn(const trek::net::Request& request);
    trek::net::Response speedUp(const trek::net::Request& request);
    trek::net::Response speedDn(const trek::net::Request& request);
    trek::net::Response voltage(const trek::net::Request& request);
    trek::net::Response amperage(const trek::net::Request& request);
    int getCell(const std::string& name);
private:
    ModulePtr mDevice;
    std::unique_ptr<Arduino> mArduino;
    FtdPtr    mFtd; 
    Config    mConfig;

    std::future<void> mMonitor;
    std::atomic_int mVoltage;
    std::atomic_bool mMonitorState;
    const int mMonitorFreq = 1000;
};


const std::string VoltageDeviceName = "/dev/hvsys";