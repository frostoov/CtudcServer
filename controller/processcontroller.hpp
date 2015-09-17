#ifndef PROCESSCONTROLLER_HPP
#define PROCESSCONTROLLER_HPP

#include <trek/common/callback.hpp>
#include <trek/net/jcontroller.hpp>

#include "managers/channelconfig.hpp"
#include "managers/processmanager.hpp"
#include "managers/frequencymanager.hpp"

class ProcessController : public trek::net::JController {
    using ProcessPtr = std::unique_ptr<caen::ProcessManager>;
    using ModulePtr  = std::shared_ptr<caen::Module>;
public:
    using Callback = trek::Callback<void(const ProcessController&)>;
public:
    class Settings {
    public:
        Settings() = default;

        nlohmann::json marshal() const;
        void unMarshal(const nlohmann::json& doc);

        uint64_t getNumberOfRun() const;
        void setNumberOfRun(const uint64_t& numberOfRun);

        uintmax_t getEventsPerFile() const;
        void setEventsPerFile(const uintmax_t& eventsPerFile);

        const std::string& getWriteDir() const;
        void setWriteDir(const std::string& writeDir);

        const std::string& getInfoPkgIp() const;
        void setInfoPkgIp(const std::string& infoPkgIp);

        uint16_t getInfoPkgPort() const;
        void setInfoPkgPort(const uint16_t& infoPkgPort);
    private:
        uint64_t    mNumberOfRun;
        uintmax_t   mEventsPerFile;
        std::string mWriteDir;
        std::string mInfoPkgIp;
        uint16_t    mInfoPkgPort;
    };
public:
    ProcessController(const ModulePtr& device,
                      const caen::ChannelConfig& config,
                      const Settings& settings);
    const std::string& name() const override;
    const Settings& getSettings() const;
    const Callback& onStopRead();
protected:
    Methods createMethods();
    trek::net::Response getType(const trek::net::Request& request);
    trek::net::Response startRead(const trek::net::Request& request);
    trek::net::Response stopRead(const trek::net::Request& request);
    trek::net::Response startFrequency(const trek::net::Request& request);
    trek::net::Response stopFrequency(const trek::net::Request& request);
    trek::net::Response getCurrentRun(const trek::net::Request& request);
    trek::net::Response getTriggerFrequency(const trek::net::Request& request) const;
    trek::net::Response getPackageFrequency(const trek::net::Request& request) const;

    ProcessPtr createReadManager(const trek::net::Request& request) const;
    bool isReadManager(const ProcessPtr& processManager) const;
    bool isFreqManager(const ProcessPtr& processManager) const;
    bool isCtudcReadManager(const ProcessPtr& processManager) const;
    nlohmann::json::array_t convertFreq(const caen::TrekFrequency& freq) const;
    nlohmann::json::array_t getProcessType(const ProcessPtr& process) const;
    std::string createWriteDir() const;
private:
    ProcessPtr          mProcess;
    ModulePtr           mDevice;
    caen::ChannelConfig mChannelConfig;
    Settings            mSettings;
    Callback            mOnStopRead;
};

#endif // PROCESSCONTROLLER_HPP
