#ifndef PROCESSCONTROLLER_HPP
#define PROCESSCONTROLLER_HPP

#include <boost/signals2.hpp>

#include "ctudccontroller.hpp"

#include "managers/channelconfig.hpp"
#include "managers/processmanager.hpp"
#include "managers/frequencymanager.hpp"

namespace ctudc {

class ProcessController;

class ProcessController : public CtudcController {
    using ProcessPtr = std::unique_ptr<caen::ProcessManager>;
    using ModulePtr  = std::shared_ptr<caen::Module>;
public:
    using StopReadSignal = boost::signals2::signal<void(const ProcessController&)>;
    using StopReadSlot   = std::function<void(const ProcessController&)>;
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
    const std::string& getName() const override;
    const Settings& getSettings() const;
    void connectStopRead(StopReadSlot&& slot);
protected:
    Methods createMethods() override;
    Response getType(const Request& request);
    Response startRead(const Request& request);
    Response stopRead(const Request& request);
    Response startFrequency(const Request& request);
    Response stopFrequency(const Request& request);
    Response getCurrentRun(const Request& request);
    Response getTriggerFrequency(const Request& request) const;
    Response getPackageFrequency(const Request& request) const;

    ProcessPtr createReadManager(const Request& request) const;
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
    const std::string   mName;
    StopReadSignal      mStopRead;
};

}

#endif // PROCESSCONTROLLER_HPP
