#ifndef FTD_MODULE_HPP
#define FTD_MODULE_HPP

#include <cstdint>
#include <string>

#include <ftd/ftd2xx.h>

#include "observer/observer.hpp"

namespace ftdi {

using string = std::string;

enum ClockRate {
    I2C_CLOCK_STANDARD_MODE		= 100000,		/* 100kb/sec */
    I2C_CLOCK_FAST_MODE			= 400000,			/* 400kb/sec */
    I2C_CLOCK_FAST_MODE_PLUS	= 1000000,		/* 1000kb/sec */
    I2C_CLOCK_HIGH_SPEED_MODE	= 3400000		/* 3.4Mb/sec */
};

struct ChannelConfig {
    ClockRate		clockRate;
    uint8_t			latencyTimer;
    uint32_t		options;
};

class Module : public Subject {
public:
    Module(uint32_t addr);
    ~Module();
    bool open(const string& desc);
    bool initialize(const ChannelConfig& conf);
    bool read(uint8_t* buff, uint32_t size);
    bool write(uint8_t* buff, uint32_t size);
    bool close();
    bool isOpen() {
        return mIsOpen;
    }

    const char* getTitle() const override {
        return "FTD";
    }
protected:
    bool doAction(const std::string& action, std::function<void()>&& func);
    void init(const ChannelConfig& conf);
    void initChannel(uint32_t clock, uchar_t timer, uint32_t options);

    void deviceRead(uint32_t sizeToTransfer, uint8_t* buffer, uint32_t& sizeTransferred, uint32_t options);
    void deviceWrite(uint32_t sizeToTransfer, uint8_t* buffer, uint32_t& sizeTransferred, uint32_t options);
    void fastRead(uint32_t sizeToTransfer, uint8_t* buffer, uint32_t& sizeTransferred, uint32_t options);
    void fastWrite(uint32_t sizeToTransfer, uint8_t* buffer, uint32_t& sizeTransferred, uint32_t options);

    void read8bitsAndGiveAck(uint8_t& data, bool ack);
    void write8bitsAndGetAck(uint8_t data, bool& ack);

    void writeDeviceAddress(bool direction, bool AddLen10Bit, bool& ack);
    void sendReceiveCmdFromMPSSE(bool echoCmdFlag, uchar_t ecoCmd, bool& cmdEchoed);
    void setGPIOLow(uint8_t value, uint8_t direction);
    void setClock(FT_DEVICE device, uint32_t clock);
    void setDeviceLoopbackState(bool loopBackFlag);

    void syncMPSSE();
    void emptyDeviceInputBuff();

    void start();
    void stop();

    void getFtDeviceType(FT_DEVICE* Module);
    bool checkMPSSEAvailable(const deviceInfo_t& devList);

    bool stat2bool(FT_STATUS status);

    void sleep(size_t time);
    void checkOpen();
private:
    ft_handle mHandle;
    uint32_t mDevAddr;
    bool mIsOpen;
    string mDescription;
};

}

#endif // FTD_MODULE_HPP
