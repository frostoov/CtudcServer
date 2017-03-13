#include "expocontroller.hpp"

#include <gsl/gsl_util.h>

#include <trek/net/request.hpp>
#include <trek/net/response.hpp>
#include <trek/net/controller.hpp>

#include <json.hpp>

using std::make_unique;
using std::string;
using std::vector;
using std::ostringstream;
using std::setw;
using std::setfill;
using std::unordered_map;
using std::function;
using std::logic_error;
using std::unique_ptr;
using std::runtime_error;
using std::chrono::microseconds;
using std::unordered_map;

using nlohmann::json;

using trek::Callback;
using trek::net::Request;
using trek::net::Response;
using trek::net::Controller;

template<typename T>
static auto convert(const T& v, const std::string& what) {
    json::array_t jFreq;
    for(auto& chamFreqPair : v) {
        auto& chamFreq = chamFreqPair.second;
        auto  chamNum  = chamFreqPair.first;
        jFreq.push_back({
            {"chamber", chamNum},
            {what,      chamFreq},
        });
    }
    return jFreq;
}

static auto convertFreq(const TrekFreq& freq) {
    return convert(freq, "freq");
}

static auto convertHitCount(const TrekHitCount& count) {
    return convert(count, "count");
}

ExpoContr::ExpoContr(const string& name,
                     const vector<DevicePtr>& devices,
                     const Settings& settings,
                     const ChannelConfig& config)
    : Controller(name, createMethods()),
      mDevices(createDeviceMap(devices)),
      mChannelConfig(config),
      mConfig(settings) { }

ExpoContr::~ExpoContr() { }

const Callback<void(unsigned)>& ExpoContr::onNewRun() {
    return mOnNewRun;
}

Controller::Methods ExpoContr::createMethods() {
    return {
        {"type",         [&](auto& request) { return this->type(request); } },
        {"run",          [&](auto& request) { return this->run(request); } },
        {"launchRead",   [&](auto& request) { return this->launchRead(request); } },
        {"stopRead",     [&](auto& request) { return this->stopRead(request); } },
        {"launchFreq",   [&](auto& request) { return this->launchFreq(request); } },
        {"stopFreq",     [&](auto& request) { return this->stopFreq(request); } },
        {"triggerCount", [&](auto& request) { return this->triggerCount(request); } },
        {"packageCount", [&](auto& request) { return this->packageCount(request); } },
        {"chambersCount",[&](auto& request) { return this->chambersCount(request); } },
        {"freq",         [&](auto& request) { return this->freq(request); } },
    };
}

Response ExpoContr::type(const Request&) {
    return {name(), __func__, {getProcessType()}};
}

Response ExpoContr::run(const Request&) {
    return {name(), __func__, {mConfig.nRun}};
}

Response ExpoContr::launchRead(const Request& request) {
    if(mFreqFuture || mExposition)
        throw logic_error("ExpoContr::launchRead process is active");
    assert(!mExposition);
    mExposition = createExposition(request.inputs);
    mOnNewRun(mConfig.nRun + 1);
    broadcast(type({}));
    broadcast(run({}));
    return {name(), __func__};
}

Response ExpoContr::stopRead(const Request&) {
    if(!mExposition)
        throw logic_error("ExpoContr::stopRead process is not active");
    assert(*mExposition);
    mExposition.reset();
    broadcast(type({}));
    ++mConfig.nRun;
    return { name(), __func__ };
}

Response ExpoContr::launchFreq(const Request& request) {
    if(mFreqFuture || mExposition)
        throw logic_error("ExpoContr::stopRead process is active");
    assert(!*mExposition);
    auto device = mDevices.at(request.inputs.at(0).get<string>());
    auto delay = request.inputs.at(1).get<int>();
    if(delay <= 0)
        throw logic_error("ExpoContr::launchFreq invalid delay value");
    mFreqFuture = ::launchFreq(device, microseconds(delay));
    broadcast(type({}));
    return { name(), __func__ };
}

Response ExpoContr::stopFreq(const Request&) {
    if(!mFreqFuture)
        throw logic_error("ExpoContr::stopFreq process is not active");
    mFreq = ::convertFreq(mFreqFuture(), mChannelConfig);
    mFreqFuture = nullptr;

    broadcast(type({}));
    return { name(), __func__ };
}

Response ExpoContr::triggerCount(const Request&) const {
    if(!mExposition)
        throw runtime_error("ExpoContr::triggerCount process is not expo");
    assert(*mExposition);
    return {name(), __func__, {mExposition->triggerCount(), mExposition->triggerDrop()}};
}

Response ExpoContr::packageCount(const Request&) const {
    if(!mExposition)
        throw runtime_error("ExpoContr::packageCount process is not expo");
    assert(*mExposition);
    return {name(), __func__, {mExposition->packageCount(), mExposition->packageDrop()}};
}

Response ExpoContr::chambersCount(const Request&) const {
    if(!mExposition)
        throw runtime_error("ExpoContr::packageCount process is not expo");
    assert(*mExposition);
    auto count = convertHitCount(mExposition->chambersCount());
    auto drop  = convertHitCount(mExposition->chambersDrop());
    return {name(), __func__, {count, drop}};
}

Response ExpoContr::freq(const Request&) const {
    return {name(), __func__, convertFreq(mFreq)};
}

string ExpoContr::getProcessType() const {
    if(mExposition) {
        assert(*mExposition);
        return "expo";
    }
    if(mFreqFuture) {
        assert(!mExposition);
        return "freq";
    }
    assert(!mExposition);
    assert(!mFreqFuture);
    return "idle";
}
unique_ptr<Exposition> ExpoContr::createExposition(const json::array_t& inputs) {
    auto expoType = inputs.at(0).get<string>();
    auto deviceName = inputs.at(1).get<string>();
    auto device = mDevices.at(deviceName);
    if(expoType == "nevod") {
        return make_unique<NevodExposition>(device, mConfig, mChannelConfig, [this](TrekFreq freq) {
            broadcast({name(), "monitoring", {convertFreq(freq)}});
        });
    } else if(expoType == "ihep") {
        if(inputs.size() == 3) 
            mConfig.readFreq = inputs.at(2).get<uintmax_t>();
        return make_unique<IHEPExposition>(device, mConfig, mChannelConfig);
    }
    throw runtime_error("ExpoContr::createExposition invalid type name");
}

auto ExpoContr::createDeviceMap(const vector<DevicePtr>& devs) -> DeviceMap {
    DeviceMap devMap;
    for(auto& d : devs) {
        devMap.emplace(d->name(), d);
    }
    return devMap;
}


void ExpoContr::Settings::unMarshal(const json& doc) {
    nRun = doc.at("number_of_run");
    eventsPerFile = doc.at("events_per_file");
    writeDir = doc.at("write_dir").get<string>();
    infoIP = doc.at("info_pkg_ip").get<string>();
    infoPort = doc.at("info_pkg_port");
    ctrlIP = doc.at("ctrl_pkg_ip");
    ctrlPort = doc.at("ctrl_pkg_port");
    readFreq = doc.at("read_freq").get<unsigned>();
	debug = doc.at("debug").get<bool>();
}

json ExpoContr::Settings::marshal() const {
    return {
        {"number_of_run", nRun},
        {"events_per_file", eventsPerFile},
        {"write_dir", writeDir},
        {"info_pkg_ip", infoIP},
        {"info_pkg_port", infoPort},
        {"ctrl_pkg_ip", ctrlIP},
        {"ctrl_pkg_port", ctrlPort},
        {"read_freq", readFreq},
		{"debug", debug},
    };
}

ExpoContr::Settings::operator NevodExposition::Settings() const {
    return {
        nRun,
        eventsPerFile,
        writeDir,
        infoIP,
        infoPort,
        ctrlIP,
        ctrlPort,
		debug,
    };
}

ExpoContr::Settings::operator IHEPExposition::Settings() const {
    return {
        nRun,
        eventsPerFile,
        writeDir,
        readFreq,
    };
}
