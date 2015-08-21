#include <thread>
#include <stdexcept>

#include <CAENVME/CAENVMElib.h>

#include "tdcmodule.hpp"

using std::unique_ptr;
using std::runtime_error;
using std::exception;
using std::string;
using trekdata::EdgeDetection;
using trekdata::Lsb;
using trekdata::Settings;

static constexpr uint16_t hShakeWo = 1;
static constexpr uint16_t hShakeRo = 2;

namespace caen {

using std::chrono::high_resolution_clock;
using std::chrono::nanoseconds;
using std::chrono::duration_cast;
using std::this_thread::sleep_for;
using std::string;

Module::Module(uint16_t vmeAddress)
    : mIsInit(false),
      mIsBlocked(false),
      mVmeHandle(0),
      mBaseAddress(vmeAddress),
      mSettings(getDefaultSettings()) { }

Module::~Module() {
    CAENVME_End(mVmeHandle);
}

bool Module::isInit() const {
    return mIsInit;
}

bool Module::isBlocked() const {
    return mIsBlocked;
}

void Module::setBlocked(bool flag) {
    mIsBlocked = flag;
}

const char* Module::getTitle() const {
    return "CAENVME Tdc";
}

Settings Module::getDefaultSettings() {
    Settings settings;
    settings.setTriggerMode(false);
    settings.setTriggerSubtraction(false);
    settings.setTdcMeta(true);
    settings.setWindowWidth(500);
    settings.setWindowOffset(-1000);
    settings.setLsb(Lsb::ps100);
    settings.setEdgeDetection(EdgeDetection::leading);
    settings.setControlRegister(32);
    settings.setStatusRegister(0);
    settings.setAlmostFull(64);
    settings.setDeadTime(0);
    settings.setEventBLT(0);
    return settings;
}

bool Module::initialize() {
    return doAction("Init", [&]() {
        CVErrorCodes status;
        if(mIsInit)
            throw std::logic_error("CAENVME already init");
        status = CAENVME_Init(cvV2718, 0, 0, &mVmeHandle);
        if(status == cvSuccess) {
            mIsInit = true;
            mSettings = getDefaultSettings();
        } else {
            mIsInit = false;
            throw runtime_error(CAENVME_DecodeError(status));
        }
        updateSettings();
    });

}

bool Module::close() {
    return doAction("Close", [&]() {
        if(mIsInit) {
            auto status = CAENVME_End(mVmeHandle);
            if(status == cvSuccess)
                mIsInit = false;
            else
                throw runtime_error(CAENVME_DecodeError(status));
        }
    });
}

bool Module::setSettings(const Settings& settings) {
    auto status = true;
    status &= setTriggerMode(settings.getTriggerMode());
    status &= setTriggerSubtraction(settings.getTriggerSubtraction());
    status &= setTdcMeta(settings.getTdcMeta());
    status &= setWindowWidth(settings.getWindowWidth());
    status &= setWindowOffset(settings.getWindowOffset());
    status &= setAlmostFull(settings.getAlmostFull());
    status &= setLsb(settings.getLsb());
    status &= setEdgeDetection(settings.getEdgeDetection());
    status &= setControl(settings.getControl());
    status &= setEventBLT(settings.getEventBLT());
    status &= setDeadTime(settings.getDeadTime());
    return status;
}

bool Module::updateSettings() {
    auto status = true;
    status &= updateAlmostFull();
    status &= updateControl();
    status &= updateStatus();
    status &= updateMode();
    status &= updateLSB();
    status &= updateDetection();
    status &= updateDeadTime();
    status &= updateTriggerConfig();
    status &= updateEventBLT();
    status &= updateTdcMeta();
    return status;
}

const trekdata::Settings& Module::getSettings() const {
    return mSettings;
}

uint32_t Module::formAddress(Reg addr) const {
    return ((uint32_t(mBaseAddress)) << 16) | uint16_t(addr);
}

bool Module::doAction(string&& message, std::function<void()>&& func) {
    try {
        func();
        pushMessage(message + ": success");
        return true;
    } catch(const exception& e) {
        pushMessage(message + ": " + e.what());
        return false;
    }
}

bool Module::setLsb(trekdata::Lsb lsb) {
    if(lsb != mSettings.getLsb())
        return doAction("Set LSB", [&]() {
        auto value = static_cast<uint16_t>(lsb);
        if(value > 2)
            throw runtime_error(CAENVME_DecodeError(cvInvalidParam));
        else {
            writeMicro(&value, OpCode::setLSB);
            mSettings.setLsb(lsb);
        }
    });
    else
        return true;
}

bool Module::setWindowWidth(uint16_t windowWidth) {
    windowWidth = (windowWidth / 25) * 25;
    if(windowWidth != mSettings.getWindowWidth())
        return doAction("Set window width", [&]() {
        uint16_t value = windowWidth / 25;
        if(value < 1 || value > 4095)
            throw runtime_error(CAENVME_DecodeError(cvInvalidParam));
        else {
            writeMicro(&value, OpCode::setWinWidth);
            mSettings.setWindowWidth(windowWidth);
        }
    });
    else
        return true;
}

bool Module::setWindowOffset(int16_t windowOffset) {
    windowOffset = (windowOffset / 25) * 25;
    if(windowOffset != mSettings.getWindowOffset())
        return doAction("Set window offset", [&]() {
        uint16_t value = static_cast<uint16_t>(windowOffset / 25);
        if(static_cast<int16_t>(value) < -2048 || static_cast<int16_t>(value) > 40)
            throw runtime_error(CAENVME_DecodeError(cvInvalidParam));
        else {
            writeMicro(&value, OpCode::setWinOffset);
            mSettings.setWindowOffset(windowOffset);
        }
    });
    else
        return true;
}

bool Module::setAlmostFull(uint16_t value) {
    if(value != mSettings.getAlmostFull())
        return doAction("Set almost Full", [&]() {
        if(value < 1 || value > 32735)
            throw runtime_error(CAENVME_DecodeError(cvInvalidParam));
        else {
            writeReg16(value, Reg::almostFull);
            mSettings.setAlmostFull(value);
        }
    });
    else
        return true;
}

bool Module::setEdgeDetection(EdgeDetection edgeDetection) {
    if(edgeDetection != mSettings.getEdgeDetection())
        return doAction("Set edge detection", [&]() {
        auto value = static_cast<uint16_t>(edgeDetection);
        if(value > 4)
            throw runtime_error(CAENVME_DecodeError(cvInvalidParam));
        else {
            writeMicro(&value, OpCode::setDetection);
            mSettings.setEdgeDetection(edgeDetection);
        }
    });
    else
        return true;
}

bool Module::setControl(uint16_t control) {
    if(control != mSettings.getControl())
        return doAction("Set control register", [&]() {
        writeReg16(control, Reg::controlReg);
        mSettings.setControlRegister(control);
    });
    else
        return true;
}

bool Module::setDeadTime(uint16_t deadTime) {
    if(deadTime != mSettings.getDeadTime())
        return doAction("Set dead time", [&]() {
        writeMicro(&deadTime, OpCode::setDeadTime);
        mSettings.setDeadTime(deadTime);
    });
    else
        return true;
}

bool Module::setEventBLT(uint16_t eventBLT) {
    if(eventBLT == mSettings.getEventBLT())
        return doAction("Set event alignment", [&]() {
        if(eventBLT > 255)
            throw runtime_error(CAENVME_DecodeError(cvInvalidParam));
        else {
            writeReg16(eventBLT, Reg::eventBLT);
            mSettings.setEventBLT(eventBLT);
        }
    });
    else
        return true;
}

bool Module::setTriggerMode(bool flag) {
    if(flag != mSettings.getTriggerMode()) {
        auto status = doAction("Set Trigger Mode", [&]() {
            if(flag) {
                writeMicro(nullptr, OpCode::setTriggerMatching);
                mSettings.setTriggerMode(true);
            } else {
                writeMicro(nullptr, OpCode::setContinuousStorage);
                mSettings.setTriggerMode(false);
            }
        });
        if(flag == false)
            status &= setTriggerSubtraction(false);
        return status;
    } else
        return true;
}

bool Module::setTriggerSubtraction(bool flag) {
    if(flag != mSettings.getTriggerSubtraction())
        return doAction("Set Trigger subtraction", [&]() {
        if(flag && !mSettings.getTriggerMode())
            throw runtime_error(CAENVME_DecodeError(cvInvalidParam));
        if(flag)
            writeMicro(nullptr, OpCode::enableSubTrig);
        else
            writeMicro(nullptr, OpCode::disableSubTrig);
        mSettings.setTriggerSubtraction(flag);
    });
    else
        return true;
}

bool Module::setTdcMeta(bool flag) {
    if(flag != mSettings.getTdcMeta())
        return doAction("Set Tdc metdata", [&]() {
        if(flag)
            writeMicro(nullptr, OpCode::enableTdcMeta);
        else
            writeMicro(nullptr, OpCode::disableTdcMeta);
        mSettings.setTdcMeta(flag);
    });
    else
        return true;
}

bool Module::updateMode() {
    return doAction("Get Trigger Mode", [&] {
        uint16_t value;
        readMicro(&value, OpCode::getMode);
        mSettings.setTriggerMode(value);
    });
}

bool Module::updateTdcMeta() {
    return doAction("Get Tdc Meta", [&] {
        uint16_t value;
        readMicro(&value, OpCode::readTdcMeta);
        mSettings.setTdcMeta(value);
    });
}

bool Module::updateDetection() {
    return doAction("Get Edge Detection", [&] {
        uint16_t value;
        readMicro(&value, OpCode::getDetection);
        mSettings.setEdgeDetection(static_cast<EdgeDetection>(value));
    });
}

bool Module::updateDeadTime() {
    return doAction("Get DeadTime", [&] {
        uint16_t value;
        readMicro(&value, OpCode::getDeadTime);
        mSettings.setDeadTime(value);
    });
}

bool Module::updateLSB() {
    return doAction("Get Lsb", [&] {
        uint16_t value;
        readMicro(&value, OpCode::getLSB);
        mSettings.setLsb(static_cast<Lsb>(value));
    });
}

uint16_t Module::getFirmwareRev() {
    return readReg16(Reg::firmwareRev);
}

uint16_t Module::getMicroRev() {
    uint16_t rev;
    readMicro(&rev, OpCode::microRev);
    return rev;
}

bool Module::updateStatus() {
    return doAction("Get Status Register", [&] {
        mSettings.setStatusRegister(readReg16(Reg::statusReg));
    });
}

bool Module::updateControl() {
    return doAction("Get Status Register", [&] {
        mSettings.setControlRegister(readReg16(Reg::controlReg));
    });
}

bool Module::updateAlmostFull() {
    return doAction("Get AlmostFull", [&] {
        mSettings.setAlmostFull(readReg16(Reg::almostFull));
    });
}

bool Module::updateEventBLT() {
    return doAction("Get Event BLT Number", [&] {
        mSettings.setEventBLT(readReg16(Reg::eventBLT));
    });
}

bool Module::updateTriggerConfig() {
    return doAction("Get Trigger conf", [&] {
        uint16_t conf[5];
        readMicro(conf, OpCode::getTrigConf, 5);
        mSettings.setWindowWidth(conf[0] * 25);
        mSettings.setWindowOffset(conf[1] * 25);
        mSettings.setTriggerSubtraction(conf[4]);
    });
}

bool Module::softwareClear() {
    return doAction("Software Clear", [&] {
        writeReg16(1, Reg::softwareClear);
    });
}

size_t Module::readBlock(WordVector& buff, const Microseconds& delay) {
    const int blockSize = getBlockSize();
    const uint32_t buffAddr = formAddress(Reg::outputBuffer);
    const uint32_t restAddr = formAddress(Reg::softwareClear);
    size_t readSize = 0 , dummy;
    int readBytes;

    auto prevSize = buff.size();
    buff.resize(prevSize + blockSize);

    if(delay != Microseconds::zero()) {
        CAENVME_WriteCycle(mVmeHandle, restAddr, &dummy, cvA32_S_DATA, cvD16);
        sleep_for(delay);
    }
    auto errCode = CAENVME_BLTReadCycle(mVmeHandle, buffAddr,
                                        reinterpret_cast<void*>(buff.data() + prevSize),
                                        blockSize * sizeof(uint32_t),
                                        cvA32_U_BLT, cvD32, &readBytes);
    if((errCode == cvBusError && (mSettings.getControl() & 1)) || errCode == cvSuccess) {
        readSize += readBytes / sizeof(uint32_t);
        buff.resize(prevSize + readBytes / sizeof(uint32_t));
    } else
        buff.resize(prevSize);
    return readSize;
}

size_t Module::getBlockSize() const {
    return 1024;
}

uint32_t Module::readReg32(Reg addr) const {
    if(mIsInit) {
        uint32_t data = 0;
        auto code = CAENVME_ReadCycle(mVmeHandle,
                                      formAddress(addr),
                                      reinterpret_cast<void*>(&data),
                                      cvA32_S_DATA, cvD32);
        if(code == cvSuccess)
            return data;
        else
            throw runtime_error(CAENVME_DecodeError(code));
    } else
        throw runtime_error(CAENVME_DecodeError(cvCommError));
}

uint16_t Module::readReg16(Reg addr) const {
    if(mIsInit) {
        uint16_t data = 0;
        auto code = CAENVME_ReadCycle(mVmeHandle,
                                      formAddress(addr),
                                      reinterpret_cast<void*>(&data),
                                      cvA32_S_DATA, cvD16);
        if(code == cvSuccess)
            return data;
        else
            throw runtime_error(CAENVME_DecodeError(code));
    } else
        throw runtime_error(CAENVME_DecodeError(cvCommError));
}

void Module::writeReg32(uint32_t data, Reg addr) {
    if(mIsInit && !mIsBlocked) {
        auto code = CAENVME_WriteCycle(mVmeHandle,
                                       formAddress(addr),
                                       reinterpret_cast<void*>(&data),
                                       cvA32_S_DATA, cvD32);
        if(code != cvSuccess)
            throw runtime_error(CAENVME_DecodeError(code));
    } else
        throw runtime_error(CAENVME_DecodeError(cvCommError));
}

void Module::writeReg16(uint16_t data, Reg addr) {
    if(mIsInit && !mIsBlocked) {
        auto code = CAENVME_WriteCycle(mVmeHandle,
                                       formAddress(addr),
                                       reinterpret_cast<void*>(&data),
                                       cvA32_S_DATA, cvD16);
        if(code != cvSuccess)
            throw runtime_error(CAENVME_DecodeError(code));
    } else
        throw runtime_error(CAENVME_DecodeError(cvCommError));
}

void Module::readMicro(uint16_t* data, OpCode code, short num) {
    if(mIsInit) {
        checkMicroWrite();
        //Указываем opCode
        writeReg16(uint16_t(code), Reg::micro);
        if(num && data) {
            do {
                try {
                    checkMicroRead();
                    *data = readReg16(Reg::micro);
                    ++data;
                } catch(const exception& e) {
                    pushMessage("Failed read micro");
                    initialize();
                    throw e;
                }
            } while(--num);
        }
    } else
        throw runtime_error(CAENVME_DecodeError(cvCommError));
}

void Module::writeMicro(uint16_t* data, OpCode code, short num) {
    if(mIsInit) {
        checkMicroWrite();
        //Указываем opCode
        writeReg16(uint16_t(code), Reg::micro);
        if(num && data) {
            do {
                //Проверяем handshake
                try {
                    checkMicroWrite();
                    writeReg16(*data, Reg::micro);
                    ++data;
                } catch(const exception& e) {
                    pushMessage("Failed write micro");
                    initialize();
                    throw e;
                }
            } while(--num);
        }
    } else
        throw runtime_error(CAENVME_DecodeError(cvCommError));
}

void Module::checkMicroRead() {
    Microseconds timeout(500);
    for(auto i = 0; i < 20000; ++i) {
        try {
            auto hShake = readReg16(Reg::handshake);
            if(hShake & hShakeRo)
                return;
        } catch(...) {}
        sleep_for(timeout);
    }
    throw runtime_error(CAENVME_DecodeError(cvTimeoutError));
}

void Module::checkMicroWrite() {
    Microseconds timeout(500);
    for(auto i = 0; i < 20000; ++i) {
        try {
            auto hShake = readReg16(Reg::handshake);
            if(hShake & hShakeWo)
                return;
        } catch(...) {}
        sleep_for(timeout);
    }
    throw runtime_error(CAENVME_DecodeError(cvTimeoutError));
}

}  // caen
