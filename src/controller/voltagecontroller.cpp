#include "voltagecontroller.hpp"

#include <boost/asio/serial_port.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>

#include <sstream>

namespace asio = boost::asio;

using std::string;
using std::logic_error;
using std::runtime_error;
using std::make_unique;

using nlohmann::json;

using trek::net::Request;
using trek::net::Response;
using trek::net::Controller;

// trim from start
static inline std::string& ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string& rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string& trim(std::string& s) {
    return ltrim(rtrim(s));
}

class VoltageContr::Arduino {
    struct Response {
        int status;
        std::string command;
        std::vector<std::string> args;

        Response(const string& str) {
            parse(str);
        }
        void parse(const string& str) {
            command.clear();
            args.clear();

            auto beg = str.cbegin();
            auto end = str.cend();

            auto it = std::find(beg, end, ' ');
            string lexem(beg, it);
            trim(lexem);
            status = std::stoi(lexem);
            if(it == end)
                return;
            beg = it + 1;
            it = std::find(beg, end, ':');
            if(it == end)
                throw std::runtime_error("Voltage::Response::parse invalid command");
            lexem.assign(beg, it);
            trim(lexem);
            command = lexem;
            if(it == end)
                return;
            beg = it + 1;
            do {
                it = std::find(beg, end, ',');
                auto arg = string{beg, it};
                if(trim(arg).empty())
                    throw std::runtime_error("Voltage::Response::parse invalid arg");
                args.push_back(arg);
                beg = it + 1;
            } while (it != end);
        }
    };
public:
    Arduino() : mPort(mIoService), mState(false) { }
    ~Arduino() {
        if(isOpen())
            close();
    }
    void open(const string& devname) {
        std::lock_guard<std::mutex> lk(mStateMutex);
        mPort.open(devname);
        mPort.set_option(asio::serial_port::baud_rate(baud_rate));
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    void close() {
        std::lock_guard<std::mutex> lk(mStateMutex);
        mPort.close();
    }
    bool isOpen() const {
        return mPort.is_open();
    }
    void turnOn() {
        setDigital(pin_no, "HIGH");
        mState = true;
    }
    void turnOff() {
        setDigital(pin_no, "LOW");
        mState = false;
    }
    bool isOn() const {
        return mState;
    }
protected:
    string send(const string& msg) {
        asio::streambuf buffer;
        {
            std::lock_guard<std::mutex> lk(mMutex);
            asio::write(mPort, asio::buffer(msg));
            asio::read_until(mPort, buffer, '\n');
        }
        std::istream istr(&buffer);
        string response;
        std::getline(istr, response);
        return response;
    }
    
    void setDigital(unsigned pin, const string& level) {
        if (!isOpen())
            return;
        std::ostringstream stream;
        stream << "digitalWrite:" << pin << ',' << level << '\n';
        Response response(send(stream.str()));
        if(response.status != 0)
            throw runtime_error("Arduino::setDigital failed set digital");
        if(response.command != "digitalWrite" || stoul(response.args.at(0)) != pin || response.args.at(1) != level)
            throw runtime_error("Arduino::setDigital invalid response");
    }
private:
    asio::io_service mIoService;
    asio::serial_port mPort;
    bool mState;

    std::mutex mStateMutex;
    std::mutex mMutex;
    static const int baud_rate = 115200;
    static const int pin_no = 4;
};

VoltageContr::VoltageContr(const string& name, const ModulePtr& module, const FtdPtr& ftd, const Config& config)
    : Controller(name, createMethods()),
      mDevice(module),
      mArduino(make_unique<Arduino>()),
      mFtd(ftd),
      mConfig(config),
      mVoltage(0),
      mMonitorState(false) {
    try {
        mArduino->open("/dev/rele");
        mArduino->turnOff();
    } catch(std::exception& e) {
        std::cerr << "Failed open arduino" << std::endl;
    }
    if(mDevice->isOpen()) {
        mMonitor = launchMonitoring();
    }
}

VoltageContr::~VoltageContr() {
    mMonitorState = false;
    if(mMonitor.valid())
        mMonitor.get();
}


Controller::Methods VoltageContr::createMethods() {
    return {
        {"open",        [&](auto& request) { return this->open(request); } },
        {"close",       [&](auto& request) { return this->close(request); } },
        {"isOpen",      [&](auto& request) { return this->isOpen(request); } },
        {"turnOn",      [&](auto& request) { return this->turnOn(request);  } },
        {"turnOff",     [&](auto& request) { return this->turnOff(request); } },
        {"stat",        [&](auto& request) { return this->stat(request); } },
        {"setVoltage",  [&](auto& request) { return this->setVoltage(request); } },
        {"setSpeedUp",  [&](auto& request) { return this->setSpeedUp(request); } },
        {"setSpeedDn",  [&](auto& request) { return this->setSpeedDn(request); } },
        {"speedUp",     [&](auto& request) { return this->speedUp(request); } },
        {"speedDn",     [&](auto& request) { return this->speedDn(request); } },
        {"voltage",     [&](auto& request) { return this->voltage(request); } },
        {"amperage",    [&](auto& request) { return this->amperage(request); } },
    };
}


Response VoltageContr::open(const Request& request) {
    mDevice->open(VoltageDeviceName);
    broadcast(isOpen({}));
    if(!mMonitor.valid())
        mMonitor = launchMonitoring();
    return {name(), __func__};
}

Response VoltageContr::close(const Request& request) {
    if(mMonitor.valid()) {
        mMonitorState = false;
        mMonitor.get();
    }
    mDevice->close();
    broadcast(isOpen({}));
    return {name(), __func__};
}

Response VoltageContr::isOpen(const Request& request) {
    return {name(), __func__, {mDevice->isOpen()}};
}

Response VoltageContr::stat(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    auto stat = mDevice->stat(cell);
    return {name(), __func__, {request.inputs.at(0), stat.value()}};
}

Response VoltageContr::turnOn(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    mDevice->turnOn(cell);
    broadcast(stat(request));
    
    return {name(), __func__};
}

Response VoltageContr::turnOff(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    mDevice->turnOff(cell);
    broadcast(stat(request));
    return {name(), __func__};
}

Response VoltageContr::setVoltage(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    auto val  = request.inputs.at(1).get<int>();
    if(cell == -1) {
        uint8_t data[] = { 0x0, 255, 0x1, uint8_t(val) };
        mFtd->write(data, sizeof(data));
    } else {
        mDevice->setVoltage(cell, val);
    }
    broadcast(voltage(request));
    return {name(), __func__};
}

Response VoltageContr::setSpeedUp(const Request& request) {
    auto cell = getCell(request.inputs.at(0));
    auto val = request.inputs.at(1).get<int>();
    mDevice->setSpeedUp(cell, val);
    broadcast(speedUp(request));
    return {name(), __func__};
}
Response VoltageContr::setSpeedDn(const Request& request) {
    auto cell = getCell(request.inputs.at(0));
    auto val = request.inputs.at(1).get<int>();
    mDevice->setSpeedDn(cell, val);
    broadcast(speedDn(request));
    return {name(), __func__};
}

Response VoltageContr::speedUp(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    return {name(), __func__, {request.inputs.at(0), mDevice->speedUp(cell)}};
}
Response VoltageContr::speedDn(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    return {name(), __func__, {request.inputs.at(0), mDevice->speedDn(cell)}};
}

Response VoltageContr::voltage(const Request& request) {
    if(!mDevice->isOpen())
        throw logic_error("VoltageContr::voltage device is closed");
    auto cell = getCell( request.inputs.at(0) );
    auto voltage = mVoltage.load();
    if(cell == mConfig.signal)
        voltage = mDevice->voltage(cell);
    return {name(), __func__, {request.inputs.at(0), voltage}};
}

Response VoltageContr::amperage(const Request& request) {
    auto cell = getCell( request.inputs.at(0) );
    return {name(), __func__, {request.inputs.at(0), mDevice->amperage(cell)}};
}

int VoltageContr::getCell(const std::string& name) {
    if(name == "signal")
        return this->mConfig.signal;
    if(name == "drift")
        return this->mConfig.drift;
    throw logic_error("VoltageContr::getCell invalid cell");
}

json VoltageContr::Config::marshal() const {
    return {
        {"signal", signal},
            {"drift", drift},
                };
}
void VoltageContr::Config::unMarhsal(const json& json) {
    signal = json.at("signal").get<int>();
    drift = json.at("drift").get<int>();
}

std::future<void> VoltageContr::launchMonitoring() {
    return std::async(std::launch::async, [&] {
        mMonitorState = true;
        while(mMonitorState) {
            try {
                bool on = false;
                mVoltage = mDevice->voltage(mConfig.drift);
                if(mVoltage > 11900)
                    on = true;
                else if(mVoltage > 200)
                    on = !mArduino->isOn();
                if(on != mArduino->isOn()) {
                    if(on)
                        mArduino->turnOn();
                    else
                        mArduino->turnOff();
                }
            } catch(std::exception& e) {
                std::cerr << "ATTENTION!!! voltage monitor loop " << e.what() << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(mMonitorFreq));
        }
    });

}
