#include "processcontroller.hpp"

#include "managers/ctudcreadmanager.hpp"

namespace ctudc {

using std::make_unique;
using std::chrono::microseconds;
using std::string;
using std::ostringstream;
using std::setw;
using std::setfill;
using std::unordered_map;
using std::function;

using nlohmann::json;

using caen::FrequencyManager;
using caen::CtudcReadManager;
using caen::ReadManager;
using caen::ProcessManager;

ProcessController::ProcessController(const ModulePtr& device,
                                     const caen::ChannelConfig& config,
                                     const Settings& settings)
    : CtudcController(createMethods()),
      mDevice(device),
      mChannelConfig(config),
      mSettings(settings),
      mName("process") { }

const std::string& ProcessController::getName() const {
    return mName;
}

const ProcessController::Settings& ProcessController::getSettings() const {
    return mSettings;
}

void ProcessController::connectStopRead(StopReadSlot&& slot) {
    mStopRead.connect(slot);
}

CtudcController::Methods ProcessController::createMethods() {
    return {
        {"getType",             [&](const Request& request) { return this->getType(request);} },
        {"startRead",           [&](const Request& request) { return this->startRead(request);} },
        {"stopRead",            [&](const Request& request) { return this->stopRead(request);} },
        {"startFrequency",      [&](const Request& request) { return this->startFrequency(request);}},
        {"stopFrequency",       [&](const Request& request) { return this->stopFrequency(request);}},
        {"getTriggerFrequency", [&](const Request& request) { return this->getTriggerFrequency(request);}},
        {"getPackageFrequency", [&](const Request& request) { return this->getPackageFrequency(request);}},
        {"getCurrentRun",       [&](const Request& request) { return this->getCurrentRun(request);}},
    };
}

CtudcController::Response ProcessController::getType(const Request& request) {
    return {
        getName(),
        "getType",
        getProcessType(mProcess),
        true
    };
}

CtudcController::Response ProcessController::startRead(const Request& request) {
    auto status = false;
    if(!mProcess) {
        mProcess = createReadManager(request);
        status = mProcess->start();
        if(!status)
            mProcess.reset();
    }
    return {
        getName(),
        "startRead",
        json::array(),
        status,
    };
}

CtudcController::Response ProcessController::stopRead(const Request& request) {
    auto status = false;
    if(isReadManager(mProcess) || isCtudcReadManager(mProcess)) {
        mProcess->stop();
        mProcess.reset();
        mSettings.setNumberOfRun(mSettings.getNumberOfRun() + 1);
        mStopRead(*this);
        status = true;
    }
    return {
        getName(),
        "stopRead",
        json::array(),
        status
    };
}

CtudcController::Response ProcessController::startFrequency(const Request& request) {
    auto status = false;
    if(!mProcess) {
        mProcess = make_unique<FrequencyManager> (mDevice, mChannelConfig, microseconds(100));
        status = mProcess->start();
        if(!status)
            mProcess.reset();
    }
    return {
        getName(),
        "startFrequency",
        json::array(),
        status
    };
}

CtudcController::Response ProcessController::stopFrequency(const Request& request) {
    auto responseStatus = false;
    caen::TrekFrequency trekFreq;
    if(isFreqManager(mProcess)) {
        mProcess->stop();
        auto freqManager = dynamic_cast<FrequencyManager*>(mProcess.get());
        if(freqManager) {
            trekFreq = freqManager->getFrequency();
            responseStatus = true;
        }
        mProcess.reset();
    }
    return {
        getName(),
        "stopFrequency",
        convertFreq(trekFreq),
        responseStatus
    };
}

CtudcController::Response ProcessController::getCurrentRun(const Request& request) {
    return {
        getName(),
        "getCurrentRun",
        json::array({mSettings.getNumberOfRun() }),
        true
    };
}

CtudcController::Response ProcessController::getTriggerFrequency(const Request& request) const {
    double freq = -1;
    auto resposeStatus = false;
    if(isCtudcReadManager(mProcess)) {
        auto ctudcManager = dynamic_cast<CtudcReadManager*>(mProcess.get());
        freq = ctudcManager->getTriggerFrequency();
        resposeStatus = true;
    }
    return {
        getName(),
        "getTriggerFrequency",
        json::array({freq}),
        resposeStatus
    };
}

CtudcController::Response ProcessController::getPackageFrequency(const Request& request) const {
    double freq = -1;
    auto resposeStatus = false;
    if(isCtudcReadManager(mProcess)) {
        auto ctudcManager = dynamic_cast<CtudcReadManager*>(mProcess.get());
        freq = ctudcManager->getPackageFrequency();
        resposeStatus = true;
    }
    return {
        getName(),
        "getPackageFrequency",
        json::array({freq}),
        resposeStatus
    };
}

ProcessController::ProcessPtr ProcessController::createReadManager(const Request& request) const {
    const auto type = request.getInputs().at(0).get<string>();
    if(type == "simple") {
        return make_unique<ReadManager> (mDevice,
                                         mChannelConfig,
                                         createWriteDir(),
                                         mSettings.getNumberOfRun(),
                                         mSettings.getEventsPerFile());
    } else if(type == "ctudc") {
        caen::CtudcReadManager::NetInfo netInfo{
            mSettings.getInfoPkgIp(), mSettings.getInfoPkgPort()
        };
        return make_unique<CtudcReadManager> (mDevice,
                                              mChannelConfig,
                                              createWriteDir(),
                                              mSettings.getNumberOfRun(),
                                              mSettings.getEventsPerFile(),
                                              netInfo);
    } else
        throw std::runtime_error("Invalid Request");
}

bool ProcessController::isReadManager(const ProcessPtr& processManager) const {
    if(!processManager)
        return false;
    const auto& ref = *processManager.get();
    auto& processType = typeid(ref);
    return 	processType == typeid(ReadManager);
}

bool ProcessController::isFreqManager(const ProcessPtr& processManager) const {
    if(!processManager)
        return false;
    const auto& ref = *processManager.get();
    auto& processType = typeid(ref);
    return 	processType == typeid(FrequencyManager);
}

bool ProcessController::isCtudcReadManager(const ProcessPtr& processManager) const {
    if(!processManager)
        return false;
    const auto& ref = *processManager.get();
    auto& processType = typeid(ref);
    return processType == typeid(CtudcReadManager);
}

json::array_t ProcessController::convertFreq(const caen::TrekFrequency& freq) const {
    json::array_t jsonFreq;
    for(const auto& freqPair : freq) {
        const auto chamber = freqPair.first;
        const auto frequency = freqPair.second;

        jsonFreq.push_back({
            {"chamber", chamber},
            {"frequency", frequency},
        });
    }
    return jsonFreq;
}

json::array_t ProcessController::getProcessType(const ProcessController::ProcessPtr& process) const {
    if(isReadManager(process)) {
        return {"read"};
    } else if(isCtudcReadManager(process)) {
        return {"ctudc"};
    } else if(isFreqManager(process)) {
        return {"frequency"};
    } else {
        return {"null"};
    }
}

string ProcessController::createWriteDir() const {
    ostringstream stream;
    stream << mSettings.getWriteDir()
           << "/data_set_" << setw(5) << setfill('0')
           << mSettings.getNumberOfRun();
    return stream.str();
}

nlohmann::json ProcessController::Settings::marshal() const {
    return {
        {"number_of_run", getNumberOfRun() },
        {"events_per_file", getEventsPerFile() },
        {"write_dir", getWriteDir() },
        {"info_pkg_ip", getInfoPkgIp() },
        {"info_pkg_port", getInfoPkgPort() }
    };
}

void ProcessController::Settings::unMarshal(const nlohmann::json& doc) {
    setNumberOfRun(doc.at("number_of_run").get<uint64_t>());
    setEventsPerFile(doc.at("events_per_file").get<uintmax_t>());
    setWriteDir(doc.at("write_dir").get<string>());
    setInfoPkgIp(doc.at("info_pkg_ip").get<string>());
    setInfoPkgPort(doc.at("info_pkg_port").get<uint16_t>());
}
uint16_t ProcessController::Settings::getInfoPkgPort() const {
    return mInfoPkgPort;
}

void ProcessController::Settings::setInfoPkgPort(const uint16_t& infoPkgPort) {
    mInfoPkgPort = infoPkgPort;
}

const string& ProcessController::Settings::getInfoPkgIp() const {
    return mInfoPkgIp;
}

void ProcessController::Settings::setInfoPkgIp(const string& infoPkgIp) {
    mInfoPkgIp = infoPkgIp;
}

const std::string& ProcessController::Settings::getWriteDir() const {
    return mWriteDir;
}

void ProcessController::Settings::setWriteDir(const string& writeDir) {
    mWriteDir = writeDir;
}

uintmax_t ProcessController::Settings::getEventsPerFile() const {
    return mEventsPerFile;
}

void ProcessController::Settings::setEventsPerFile(const uintmax_t& eventsPerFile) {
    mEventsPerFile = eventsPerFile;
}

uint64_t ProcessController::Settings::getNumberOfRun() const {
    return mNumberOfRun;
}

void ProcessController::Settings::setNumberOfRun(const uint64_t& numberOfRun) {
    mNumberOfRun = numberOfRun;
}


}
