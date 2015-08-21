#include <stdexcept>
#include <thread>
#include <cstring>

#include "ftdmodule.hpp"
#include "defines.hpp"

static const char* decodeStatus(FT_STATUS item) {
    switch(item) {
    case FT_OK:
        return "Success";
    case FT_INVALID_HANDLE:
        return "Invalid Handle";
    case FT_DEVICE_NOT_FOUND:
        return "Device not found";
    case FT_DEVICE_NOT_OPENED:
        return "Device not open";
    case FT_IO_ERROR:
        return "I/O error";
    case FT_INSUFFICIENT_RESOURCES:
        return "Insufficient resources";
    case FT_INVALID_PARAMETER:
        return "Invalid Parameter";
    case FT_INVALID_BAUD_RATE:
        return "Invalid baud rate";
    case FT_DEVICE_NOT_OPENED_FOR_ERASE:
        return "Device not opened for erase";
    case FT_DEVICE_NOT_OPENED_FOR_WRITE:
        return "Device not opened for write";
    case FT_FAILED_TO_WRITE_DEVICE:
        return "Failed to write device";
    case FT_EEPROM_READ_FAILED:
        return "EEPROM read failed";
    case FT_EEPROM_WRITE_FAILED:
        return "EEPROM write failed";
    case FT_EEPROM_ERASE_FAILED:
        return "EEPROM erase failed";
    case FT_EEPROM_NOT_PRESENT:
        return "EEPROM not present";
    case FT_EEPROM_NOT_PROGRAMMED:
        return "EEPROM not programmed";
    case FT_INVALID_ARGS:
        return "Invalide Arguements";
    case FT_NOT_SUPPORTED:
        return "Not supported";
    case FT_OTHER_ERROR:
        return "Other error";
    default:
        return "Unknown error";
    }
}

static void handleStatus(FT_STATUS status) {
    if(status != FT_OK)
        throw std::runtime_error(decodeStatus(status));
}

namespace ftdi {

using std::runtime_error;
using std::exception;

Module::Module(uint32_t addr)
    : mHandle(nullptr), mDevAddr(addr), mIsOpen(false) {}

Module::~Module() {
    close();
}

bool Module::doAction(const std::string& action, std::function<void ()>&& func) {
    try {
        checkOpen();
        func();
        pushMessage(action + ": success");
        return true;
    } catch(const exception& e) {
        pushMessage(action + ": " + e.what());
        return false;
    }
}

bool Module::open(const string& desc) {
    uint32_t tempNumChannels;
    mDescription.clear();
    mIsOpen = false;
    /*Get the number of devices connected to the system(FT_CreateDeviceInfoList)*/
    auto status = FT_CreateDeviceInfoList(&tempNumChannels);
    if(status == FT_OK) {
        deviceInfo_t devices[tempNumChannels];
        status = FT_GetDeviceInfoList(devices, &tempNumChannels);
        if(status == FT_OK) {
            /*loop until No of devices */
            uint32_t devInd = 0;
            for(const auto& dev : devices) {
                if(checkMPSSEAvailable(dev))
                    if(strcmp(dev.Description, desc.c_str()) == 0) {
                        status = FT_Open(devInd, &mHandle);
                        if(status == FT_OK) {
                            mDescription = dev.Description;
                            mIsOpen = true;
                            break;
                        }
                    }
                ++devInd;
            }
        } else
            status = FT_INVALID_HANDLE;
    }
    if(mIsOpen == false)
        status = FT_DEVICE_NOT_FOUND;
    pushMessage(string("Open: ") + decodeStatus(status));
    return mIsOpen;
}

bool Module::initialize(const ChannelConfig& conf) {
    return doAction("Init", [&]() {
        init(conf);
    });
}

bool Module::read(uint8_t* buff, uint32_t size) {
    return doAction("Read", [&]() {
        uint32_t retCount;
        deviceRead(size, buff, retCount, 0x3);
    });
}

bool Module::write(uint8_t* buff, uint32_t size) {
    return doAction("Write", [&]() {
        uint32_t retCount;
        deviceWrite(size, buff, retCount, 0x3);
    });
}

bool Module::close() {
    return doAction("Close", [&]() {
        auto status = FT_Close(mHandle);
        handleStatus(status);
        mIsOpen = false;
    });
}

void Module::init(const ChannelConfig& conf) {
    uint32_t clock;
    if(!(conf.options & I2C_DISABLE_3PHASE_CLOCKING))
        clock = (conf.clockRate * 3) / 2;
    else
        clock = conf.clockRate;
    FT_STATUS status;
    initChannel(clock, conf.latencyTimer, conf.options);
    if(!(conf.options & I2C_DISABLE_3PHASE_CLOCKING)) {
        uint32_t retCount = 0;
        uchar_t cmd = MPSSE_CMD_ENABLE_3PHASE_CLOCKING;
        status = FT_Write(mHandle, &cmd, sizeof(cmd), &retCount);
        if(status != FT_OK)
            throw runtime_error(decodeStatus(status));
    }
    uchar_t data[16];
    uint32_t retCount;
    data[0] = 0x9E;
    data[1] = 0x03;
    data[2] = 0x0;
    status = FT_Write(mHandle, data, 3 * sizeof(uchar_t), &retCount);
    handleStatus(status);
}

bool Module::checkMPSSEAvailable(const deviceInfo_t& devList) {
    bool isMPSSEAvailable = false;
    /*check TYPE field*/
    switch(devList.Type) {
    case FT_DEVICE_2232C:
        if((devList.LocId & 0xf) == 1)
            isMPSSEAvailable =  true;
        break;
    case FT_DEVICE_2232H:
        if((devList.LocId & 0xf) == 1 || (devList.LocId & 0xf) == 2)
            isMPSSEAvailable =  true;
        break;
    case FT_DEVICE_4232H:
        if((devList.LocId & 0xf) == 1 || (devList.LocId & 0xf) == 2)
            isMPSSEAvailable =  true;
        break;
    case FT_DEVICE_232H:
        isMPSSEAvailable =  true;
        break;
    default:
        break;
    };
    return isMPSSEAvailable;
}

void Module::initChannel(uint32_t clock, uchar_t timer, uint32_t options) {
    FT_DEVICE ftDevice;

    /*Check parameters*/
    if(clock > CLOCKRATE_MAX)
        throw runtime_error(decodeStatus(FT_INVALID_PARAMETER));

    /*Get the device type*/
    getFtDeviceType(&ftDevice);
    auto status = FT_ResetDevice(mHandle);
    handleStatus(status);
    status = FT_Purge(mHandle, FT_PURGE_RX | FT_PURGE_TX);
    handleStatus(status);
    status = FT_SetUSBParameters(mHandle, usbBufferSize, usbBufferSize);
    handleStatus(status);
    status = FT_SetChars(mHandle, 0, disableEvent, 0, disableChar);
    handleStatus(status);
    status = FT_SetTimeouts(mHandle, 5000, writeTimeout);
    handleStatus(status);
    status = FT_SetLatencyTimer(mHandle, (uchar_t) timer);
    handleStatus(status);
    //Reset
    status = FT_SetBitMode(mHandle, interfaceMaskIn, resetInterface);
    handleStatus(status);
    /*EnableMPSSEInterface*/
    status = FT_SetBitMode(mHandle, interfaceMaskIn, ENABLE_MPSSE);
    handleStatus(status);
    /*20110608 - enabling loopback before sync*/
    setDeviceLoopbackState(true);
    /*Sync MPSSE */
    syncMPSSE();
    /*wait for USB*/
    sleep(50);
    /*set Clock frequency*/
    setClock(ftDevice, clock);
    sleep(20);
    /*Stop Loop back*/
    setDeviceLoopbackState(false);
    emptyDeviceInputBuff();
    /*Set i/o pin states*/
    setGPIOLow(0x13, 0x13);

    if(ftDevice == FT_DEVICE_232H &&  options & I2C_ENABLE_DRIVE_ONLY_ZERO) {
        uint8_t buffer[3];
        uint32_t noOfBytesToTransfer;
        uint32_t noOfBytesTransferred;
        noOfBytesToTransfer = 3;
        noOfBytesTransferred = 0;
        buffer[0] = MPSSE_CMD_ENABLE_DRIVE_ONLY_ZERO;/* MPSSE command */
        buffer[1] = 0x03; /* LowByte */
        buffer[2] = 0x00; /* HighByte */
        status = FT_Write(mHandle, buffer, noOfBytesToTransfer, &noOfBytesTransferred);
        handleStatus(status);
    }
}

void Module::deviceRead(uint32_t sizeToTransfer, uint8_t* buffer, uint32_t& sizeTransferred, uint32_t options) {
    if(options & I2C_TRANSFER_OPTIONS_FAST_TRANSFER)
        fastRead(sizeToTransfer, buffer, sizeTransferred, options);
    else {
        /* Write START bit */
        if(options & I2C_TRANSFER_OPTIONS_START_BIT)
            start();

        /* Write device address (with LSB=1 => READ)  & Get ACK */
        bool ack = true;
        writeDeviceAddress(true, false, ack);

        /* check acknowledgement of device address write */
        if(ack == true)
            throw runtime_error(decodeStatus(FT_DEVICE_NOT_FOUND));
        uint32_t i;
        for(i = 0; i < sizeToTransfer; ++i) {
            bool flag;
            if(i < sizeToTransfer - 1)
                flag = true;
            else {
                if(options & I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE)
                    flag = false;
                else
                    flag = true;
            }
            read8bitsAndGiveAck(buffer[i], flag);
        }

        sizeTransferred = i;
        if(sizeTransferred != sizeToTransfer)
            throw runtime_error(decodeStatus(FT_IO_ERROR));
        else {
            /* Write STOP bit */
            if(options & I2C_TRANSFER_OPTIONS_STOP_BIT)
                stop();
        }
    }
}


void Module::deviceWrite(uint32_t sizeToTransfer, uint8_t* buffer, uint32_t& sizeTransferred, uint32_t options) {
    auto status = FT_Purge(mHandle, FT_PURGE_RX | FT_PURGE_TX);
    handleStatus(status);

    if(options & I2C_TRANSFER_OPTIONS_FAST_TRANSFER)
        fastWrite(sizeToTransfer, buffer, sizeTransferred, options);
    else {
        /* Write START bit */
        if(options & I2C_TRANSFER_OPTIONS_START_BIT)
            start();

        /* Write device address (with LSB=0 => Write) & Get ACK*/
        bool ack = false;
        writeDeviceAddress(false, false, ack);

        /*ack bit set actually means device nAcked*/
        if(ack == true)
            throw runtime_error(decodeStatus(FT_DEVICE_NOT_FOUND));
        uint32_t i;
        /* LOOP until sizeToTransfer */
        for(i = 0; i < sizeToTransfer; i++) {
            /* Write byte to buffer & Get ACK */
            ack = false;
            write8bitsAndGetAck(buffer[i], ack);
            if(ack && options & I2C_TRANSFER_OPTIONS_BREAK_ON_NACK)
                throw runtime_error(decodeStatus(FT_FAILED_TO_WRITE_DEVICE));
        }
        sizeTransferred = i;
        if(sizeTransferred != sizeToTransfer)
            throw runtime_error(decodeStatus(FT_IO_ERROR));
        else {
            /* Write STOP bit */
            if(options & I2C_TRANSFER_OPTIONS_STOP_BIT)
                stop();
        }
    }
}

void Module::checkOpen() {
    if(mIsOpen == false)
        throw runtime_error(decodeStatus(FT_DEVICE_NOT_OPENED));
}

void Module::fastRead(uint32_t sizeToTransfer, uint8_t* buffer, uint32_t& sizeTransferred, uint32_t options) {
    if(sizeToTransfer == 0)
        return;
    FT_STATUS status = FT_OK;
    size_t i = 0; /* index of cmdBuffer that is filled */
    uint32_t bytesRead;
    uint8_t  addressAck;
    uint32_t bytesToTransfer;
    uint32_t bitsToTransfer;

    if(options & I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS) {
        /* size is in bits */
        bitsToTransfer = sizeToTransfer;
    } else {
        /* size is in bytes */
        bitsToTransfer = sizeToTransfer * 8;
    }
    bytesToTransfer =
        (bitsToTransfer > 0) ? (((bitsToTransfer / 8) == 0) ? 1 : (bitsToTransfer / 8)) : (0);

    /* Calculate size of required buffer */

    /* the size of data itself */
    uint32_t sizeTotal = bytesToTransfer * 12;
    /* for address byte*/
    if(!(options & I2C_TRANSFER_OPTIONS_NO_ADDRESS))
        sizeTotal += 11;
    /* size required for START */
    if(options & I2C_TRANSFER_OPTIONS_START_BIT)
        sizeTotal += (START_DURATION_1 + START_DURATION_2 + 1) * 3;
    /* size for STOP */
    if(options & I2C_TRANSFER_OPTIONS_STOP_BIT)
        sizeTotal += (STOP_DURATION_1 +	STOP_DURATION_2 + STOP_DURATION_3 + 1) * 3;

    if(!sizeTotal)
        return;
    /* Allocate buffers */
    uint8_t outBuffer[sizeTotal];

    /* Write START bit */
    if(options & I2C_TRANSFER_OPTIONS_START_BIT) {
        /* SCL high, SDA high */
        for(short j = 0; j < START_DURATION_1; j++) {
            outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
            outBuffer[i++] = VALUE_SCLHIGH_SDAHIGH;
            outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
        }
        /* SCL high, SDA low */
        for(short j = 0; j < START_DURATION_2; j++) {
            outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
            outBuffer[i++] = VALUE_SCLHIGH_SDALOW;
            outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
        }
        /*SCL low, SDA low */
        outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
        outBuffer[i++] = VALUE_SCLLOW_SDALOW;
        outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
    }

    if(!(options & I2C_TRANSFER_OPTIONS_NO_ADDRESS)) {
        auto tempAddress = (uint8_t) mDevAddr;
        tempAddress = (tempAddress << 1);

        /*set direction*/
        outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
        outBuffer[i++] = VALUE_SCLLOW_SDAHIGH; /*Value*/
        outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT; /*Direction*/

        /* write address + direction bit */
        outBuffer[i++] = MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE; /* MPSSE command */
        outBuffer[i++] = DATA_SIZE_8BITS;
        outBuffer[i++] = tempAddress;
    }

    /* add commands & data to buffer */
    size_t j = 0;
    while(j < bitsToTransfer) {
        uint8_t  bitsInThisTransfer = (bitsToTransfer - j > 8) ? 8 : bitsToTransfer - j;

        /*set direction*/
        outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
        outBuffer[i++] = VALUE_SCLLOW_SDALOW; /*Value*/
        outBuffer[i++] = DIRECTION_SCLOUT_SDAIN; /*Direction*/

        /*Command to read 8 bits*/
        outBuffer[i++] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE;
        outBuffer[i++] = bitsInThisTransfer - 1;

        /*Command MPSSE to send data to PC immediately */
        /*buffer[i++] = MPSSE_CMD_SEND_IMMEDIATE;*/

        /* Write 1bit ack after each 8bits read - only in byte mode */
        if(options & I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES) {

            /* Proposal 2*/
            outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
            outBuffer[i++] = VALUE_SCLLOW_SDALOW ;
            outBuffer[i++] = DIRECTION_SCLOUT_SDAIN;                                                                                                      //GB set SDA to in so that following data lockout has no effect, ack/nak driven by the other I/O line on SDA

            // Burn off one I2C bit time
            outBuffer[i++] = MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE;
            outBuffer[i++] = 0; /*0x00=1bit; 0x07=8bits*/
            if(j < bitsToTransfer - 1)
                outBuffer[i++] = SEND_ACK;
            else {
                if(options & I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE)
                    outBuffer[i++] = SEND_NACK;
                else
                    outBuffer[i++] = SEND_ACK;
            }

        }
        j += bitsInThisTransfer;
    }
    sizeTransferred = j;

    /* Write STOP bit */
    if(options & I2C_TRANSFER_OPTIONS_STOP_BIT) {
        /* SCL low, SDA low */
        for(j = 0 ; j < STOP_DURATION_1 ; j++) {
            outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
            outBuffer[i++] = VALUE_SCLLOW_SDALOW;
            outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
        }
        /* SCL high, SDA low */
        for(j = 0; j < STOP_DURATION_2; j++) {
            outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
            outBuffer[i++] = VALUE_SCLHIGH_SDALOW;
            outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
        }
        /* SCL high, SDA high */
        for(j = 0; j < STOP_DURATION_3; j++) {
            outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
            outBuffer[i++] = VALUE_SCLHIGH_SDAHIGH;
            outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
        }
        outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
        outBuffer[i++] = VALUE_SCLHIGH_SDAHIGH;
        outBuffer[i++] = DIRECTION_SCLIN_SDAIN; /* Tristate the SCL & SDA pins */
    }

    /* write buffer */
    status = FT_Write(mHandle, outBuffer, i, &bytesRead);
    if(status != FT_OK)
        throw runtime_error(decodeStatus(status));

    /*read the address ack bit */
    if(!(options & I2C_TRANSFER_OPTIONS_NO_ADDRESS)) {
        status = FT_Read(mHandle, &addressAck, sizeof(addressAck), &bytesRead);
        handleStatus(status);
    }

    /* read the actual data from the MPSSE-chip into the host system */
    if(options & I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES)
        status = FT_Read(mHandle, buffer, bytesToTransfer, &bytesRead);
    else
        status = FT_Read(mHandle, buffer, bytesToTransfer + 1, &bytesRead);

    handleStatus(status);
}

void Module::fastWrite(uint32_t sizeToTransfer, uint8_t* buffer, uint32_t& sizeTransferred, uint32_t options) {
    if(sizeToTransfer == 0)
        return;
    FT_STATUS status = FT_OK;
    uint32_t i = 0; /* index of cmdBuffer that is filled */
    uint32_t bytesRead;
    uint32_t bytesToTransfer;
    uint32_t bitsToTransfer;

    if(options & I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS)
        bitsToTransfer = sizeToTransfer;
    else
        bitsToTransfer = sizeToTransfer * 8;
    if(bitsToTransfer) {
        if(bitsToTransfer / 8 == 0)
            bytesToTransfer = 1;
        else
            bytesToTransfer = (bitsToTransfer / 8) + 1;
    } else
        bytesToTransfer = 0;

    /*the size of data itself */
    uint32_t sizeTotal = bytesToTransfer * 6;
    /*for address byte*/
    if(!(options & I2C_TRANSFER_OPTIONS_NO_ADDRESS))
        sizeTotal += 11;
    /* size required for START */
    if(options & I2C_TRANSFER_OPTIONS_START_BIT)
        sizeTotal += (START_DURATION_1 + START_DURATION_2 + 1) * 3;
    /* size for STOP */
    if(options & I2C_TRANSFER_OPTIONS_STOP_BIT)
        sizeTotal += (STOP_DURATION_1 + STOP_DURATION_2 + STOP_DURATION_3 + 1) * 3;

    if(!sizeTotal)
        return;
    /* Allocate buffers */
    uint8_t outBuffer[sizeTotal];

    /* Write START bit */
    if(options & I2C_TRANSFER_OPTIONS_START_BIT) {
        /* SCL high, SDA high */
        for(short j = 0; j < START_DURATION_1; j++) {
            outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
            outBuffer[i++] = VALUE_SCLHIGH_SDAHIGH;
            outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
        }
        /* SCL high, SDA low */
        for(short j = 0; j < START_DURATION_2; j++) {
            outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
            outBuffer[i++] = VALUE_SCLHIGH_SDALOW;
            outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
        }
        /*SCL low, SDA low */
        outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
        outBuffer[i++] = VALUE_SCLLOW_SDALOW;
        outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
    }

    if(!(options & I2C_TRANSFER_OPTIONS_NO_ADDRESS)) {
        auto tempAddress = (uint8_t) mDevAddr;
        tempAddress = (tempAddress << 1);
        tempAddress = (tempAddress & I2C_ADDRESS_WRITE_MASK);

        /*set direction*/
        outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
        outBuffer[i++] = VALUE_SCLLOW_SDAHIGH; /*Value*/
        outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT; /*Direction*/

        /* write address + direction bit */
        outBuffer[i++] = MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE;/* MPSSE command */
        outBuffer[i++] = DATA_SIZE_8BITS;
        outBuffer[i++] = tempAddress;
    }

    /* add commands & data to buffer */
    size_t j = 0;
    while(j < bitsToTransfer) {
        /*set direction*/
        outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
        outBuffer[i++] = VALUE_SCLLOW_SDAHIGH; /*Value*/
        outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT; /*Direction*/

        /* Command to write 8bits */
        auto bitsInThisTransfer = ((bitsToTransfer - j) > 8) ? 8 : (bitsToTransfer - j);
        outBuffer[i++] = MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE; /* MPSSE command */
        outBuffer[i++] = bitsInThisTransfer - 1;
        outBuffer[i++] = buffer[j / 8];

        j += bitsInThisTransfer;
    }

    /* Write STOP bit */
    if(options & I2C_TRANSFER_OPTIONS_STOP_BIT) {
        /* SCL low, SDA low */
        for(j = 0; j < STOP_DURATION_1; j++) {
            outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
            outBuffer[i++] = VALUE_SCLLOW_SDALOW;
            outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
        }
        /* SCL high, SDA low */
        for(j = 0; j < STOP_DURATION_2; j++) {
            outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
            outBuffer[i++] = VALUE_SCLHIGH_SDALOW;
            outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
        }
        /* SCL high, SDA high */
        for(j = 0; j < STOP_DURATION_3; j++) {
            outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
            outBuffer[i++] = VALUE_SCLHIGH_SDAHIGH;
            outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
        }
        outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
        outBuffer[i++] = VALUE_SCLHIGH_SDAHIGH;
        outBuffer[i++] = DIRECTION_SCLIN_SDAIN; /* Tristate the SCL & SDA pins */
    }

    /* write buffer */
    status = FT_Write(mHandle, outBuffer, i, &bytesRead);
    handleStatus(status);
    sizeTransferred = sizeToTransfer;
}

void Module::read8bitsAndGiveAck(uint8_t& data, bool ack) {
    uint8_t buffer[20], inBuffer[5];
    uint32_t noOfBytes = 0;
    uint32_t noOfBytesTransferred;

    /*set direction*/
    buffer[noOfBytes++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
    buffer[noOfBytes++] = VALUE_SCLLOW_SDALOW; /*Value*/
    buffer[noOfBytes++] = DIRECTION_SCLOUT_SDAIN; /*Direction*/

    /*Command to read 8 bits*/
    buffer[noOfBytes++] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE;
    buffer[noOfBytes++] = DATA_SIZE_8BITS;/*0x00=1bit; 0x07=8bits*/

    /*Command MPSSE to send data to PC immediately */
    buffer[noOfBytes++] = MPSSE_CMD_SEND_IMMEDIATE;

    /* Proposal 2*/
    buffer[noOfBytes++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
    buffer[noOfBytes++] = VALUE_SCLLOW_SDALOW;
    buffer[noOfBytes++] = DIRECTION_SCLOUT_SDAIN;

    // Burn off one I2C bit time
    buffer[noOfBytes++] = MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE;
    buffer[noOfBytes++] = 0; /*0x00=1bit; 0x07=8bits*/
    buffer[noOfBytes++] = ack ? SEND_ACK : SEND_NACK;	/*Only MSB is sent*/


    auto status = FT_Write(mHandle, buffer, noOfBytes, &noOfBytesTransferred);
    if(FT_OK != status)
        throw runtime_error(decodeStatus(status));
    else if(noOfBytes != noOfBytesTransferred)
        throw runtime_error(decodeStatus(FT_IO_ERROR));
    else {
        noOfBytes = 1;
        status = FT_Read(mHandle, inBuffer, noOfBytes, &noOfBytesTransferred);
        if(FT_OK != status)
            throw runtime_error(decodeStatus(status));
        else if(noOfBytes != noOfBytesTransferred)
            throw runtime_error(decodeStatus(FT_IO_ERROR));
        else
            data = inBuffer[0];
    }
}

void Module::write8bitsAndGetAck(uint8_t data, bool& ack) {
    uint8_t buffer[20] = {0};
    uint8_t inBuffer[3] = {0};
    uint32_t noOfBytes = 0;
    uint32_t noOfBytesTransferred;

    /*set direction*/
    buffer[noOfBytes++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
    buffer[noOfBytes++] = VALUE_SCLLOW_SDAHIGH; /*Value*/
    buffer[noOfBytes++] = DIRECTION_SCLOUT_SDAOUT; /*Direction*/

    /* Command to write 8bits */
    buffer[noOfBytes++] = MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE;/* MPSSE command */
    buffer[noOfBytes++] = DATA_SIZE_8BITS;
    buffer[noOfBytes++] = data;

    /* Set SDA to input mode before reading ACK bit */
    buffer[noOfBytes++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
    buffer[noOfBytes++] = VALUE_SCLLOW_SDALOW; /*Value*/
    buffer[noOfBytes++] = DIRECTION_SCLOUT_SDAIN; /*Direction*/

    /* Command to get ACK bit */
    buffer[noOfBytes++] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE;/* MPSSE command */
    buffer[noOfBytes++] = DATA_SIZE_1BIT; /*Read only one bit */

    /*Command MPSSE to send data to PC immediately */
    buffer[noOfBytes++] = MPSSE_CMD_SEND_IMMEDIATE;
    auto status = FT_Write(mHandle, buffer, noOfBytes, &noOfBytesTransferred);
    if(FT_OK != status)
        throw runtime_error(decodeStatus(status));
    else if(noOfBytes != noOfBytesTransferred)
        throw runtime_error(decodeStatus(FT_IO_ERROR));
    else {
        /*Get ack*/
        sleep(1);
        uint8_t ackVal = 0;
        status = FT_Read(mHandle, &ackVal, sizeof(ackVal), &noOfBytesTransferred);
        if(FT_OK != status)
            throw runtime_error(decodeStatus(status));
        else if(noOfBytesTransferred != sizeof(ackVal))
            throw runtime_error(decodeStatus(FT_IO_ERROR));
        else
            ack = (bool)(inBuffer[0] & 0x01);
    }
}


void Module::writeDeviceAddress(bool direction, bool AddLen10Bit, bool& ack) {
    if(!AddLen10Bit) {
        /* 7bit addressing */
        auto tempAddress = (uint8_t) mDevAddr;
        tempAddress = (tempAddress << 1);
        if(direction)
            tempAddress = (tempAddress | I2C_ADDRESS_READ_MASK);
        else
            tempAddress = (tempAddress & I2C_ADDRESS_WRITE_MASK);
        write8bitsAndGetAck(tempAddress, ack);
    } else
        /* 10bit addressing */
        throw runtime_error(decodeStatus(FT_NOT_SUPPORTED));
}

void Module::start() {
    uint8_t buffer[(START_DURATION_1 + START_DURATION_2 + 1) * 3];
    uint32_t i = 0;

    /* SCL high, SDA high */
    for(short j = 0 ; j < START_DURATION_1 ; j++) {
        buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
        buffer[i++] = VALUE_SCLHIGH_SDAHIGH;
        buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
    }
    /* SCL high, SDA low */
    for(short j = 0; j < START_DURATION_2; j++) {
        buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
        buffer[i++] = VALUE_SCLHIGH_SDALOW;
        buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
    }
    /*SCL low, SDA low */
    buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
    buffer[i++] = VALUE_SCLLOW_SDALOW;
    buffer[i++] = DIRECTION_SCLOUT_SDAOUT;

    uint32_t noOfBytesTransferred = 0;
    auto status = FT_Write(mHandle, buffer, i, &noOfBytesTransferred);
    handleStatus(status);
}

void Module::stop() {
    uint8_t buffer[(STOP_DURATION_1 + STOP_DURATION_2 + STOP_DURATION_3 + 1) * 3];
    uint32_t i = 0;

    /* SCL low, SDA low */
    for(short j = 0; j < STOP_DURATION_1; j++) {
        buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
        buffer[i++] = VALUE_SCLLOW_SDALOW;
        buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
    }
    /* SCL high, SDA low */
    for(short j = 0; j < STOP_DURATION_2; j++) {
        buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
        buffer[i++] = VALUE_SCLHIGH_SDALOW;
        buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
    }
    /* SCL high, SDA high */
    for(short j = 0; j < STOP_DURATION_3; j++) {
        buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
        buffer[i++] = VALUE_SCLHIGH_SDAHIGH;
        buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
    }
    buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
    buffer[i++] = VALUE_SCLHIGH_SDAHIGH;
    buffer[i++] = DIRECTION_SCLIN_SDAIN; /* Tristate the SCL & SDA pins */

    uint32_t noOfBytesTransferred = 0;
    auto status = FT_Write(mHandle, buffer, i, &noOfBytesTransferred);
    handleStatus(status);
}


void Module::setGPIOLow(uint8_t value, uint8_t direction) {
    uchar_t inputBuffer[10];
    uint32_t bytesWritten = 0;
    uint32_t bufIdx = 0;

    inputBuffer[bufIdx++] = SET_LOW_BYTE_DATA_BITS;
    inputBuffer[bufIdx++] = value;//0x13;
    inputBuffer[bufIdx++] = direction;//0x13;

    auto status = FT_Write(mHandle, inputBuffer, bufIdx, &bytesWritten);
    handleStatus(status);
}

void Module::syncMPSSE() {
    bool cmdEchoed;

    emptyDeviceInputBuff();
    /*send and receive command*/
    sendReceiveCmdFromMPSSE(true, MID_ECHO_CMD_1, cmdEchoed);

    if(cmdEchoed == true) {
        sendReceiveCmdFromMPSSE(false, MID_ECHO_CMD_2, cmdEchoed);
        if(cmdEchoed != true)
            throw runtime_error(decodeStatus(FT_OTHER_ERROR));
    } else
        throw runtime_error(decodeStatus(FT_OTHER_ERROR));
}

void Module::emptyDeviceInputBuff() {
    uint32_t bytesInInputBuf = 0;
    uint32_t bytesToRead;

    auto status = FT_GetQueueStatus(mHandle, &bytesInInputBuf);
    handleStatus(status);
    if(bytesInInputBuf == 0)
        return;

    uchar_t readBuffer[MID_MAX_IN_BUF_SIZE];
    uint32_t numOfBytesRead = 0;
    do {
        if(bytesInInputBuf > MID_MAX_IN_BUF_SIZE)
            bytesToRead = MID_MAX_IN_BUF_SIZE;
        else
            bytesToRead = bytesInInputBuf;

        status = FT_Read(mHandle, readBuffer, bytesToRead, &numOfBytesRead);
        handleStatus(status);
        if(bytesInInputBuf < numOfBytesRead)
            break;
        bytesInInputBuf -= numOfBytesRead;
    } while(bytesInInputBuf != 0);
}

void Module::setClock(FT_DEVICE device, uint32_t clock) {
    uchar_t inputBuffer[10];
    uint32_t bytesWritten = 0;
    uint32_t bufIdx = 0;
    uint8_t valueH, valueL;
    uint32_t value;
    FT_STATUS status;

    switch(device) {
    case FT_DEVICE_2232C:/* This is actually FT2232D but defined is FT_DEVICE_2232C
            in D2XX. Also, it is the only FS device that supports MPSSE */
        value = (MID_30MHZ / clock) - 1;
        break;

    default:/* Assuming all new chips will he HS MPSSE devices */
    /* Fall through */
    case FT_DEVICE_2232H:
    case FT_DEVICE_4232H:
    case FT_DEVICE_232H:
        if(clock <= MID_6MHZ) {
            value = ENABLE_CLOCK_DIVIDE;
            status = FT_Write(mHandle, &value, 1, &bytesWritten);
            handleStatus(status);
            value = (MID_6MHZ / clock) - 1;
        } else {
            value = DISABLE_CLOCK_DIVIDE;
            status = FT_Write(mHandle, &value, 1, &bytesWritten);
            handleStatus(status);
            value = (MID_30MHZ / clock) - 1;
        }
        break;
    }
    /*calculate valueH and ValueL*/
    valueL = (uint8_t) value;
    valueH = (uint8_t)(value >> 8);
    /*set the clock*/
    inputBuffer[bufIdx++] = SET_CLOCK_FREQUENCY;
    inputBuffer[bufIdx++] = valueL;
    inputBuffer[bufIdx++] = valueH;
    status = FT_Write(mHandle, inputBuffer, bufIdx, &bytesWritten);
    handleStatus(status);
}


void Module::setDeviceLoopbackState(bool loopBackFlag) {
    uchar_t inputBuffer[10];
    uint32_t bytesWritten = 0;
    uint32_t bufIdx = 0;

    if(loopBackFlag == false)
        inputBuffer[bufIdx++] = TURN_OFF_LOOPBACK;
    else
        inputBuffer[bufIdx++] = TURN_ON_LOOPBACK;

    auto status = FT_Write(mHandle, inputBuffer, bufIdx, &bytesWritten);
    handleStatus(status);
}

void Module::getFtDeviceType(FT_DEVICE* ftDevice) {
    uint32_t deviceID;
    char serialNumber[300];
    char description[300];
    auto status = FT_GetDeviceInfo(mHandle, ftDevice, &deviceID, serialNumber, description, nullptr);
    handleStatus(status);
}

void Module::sendReceiveCmdFromMPSSE(bool echoCmdFlag, uchar_t ecoCmd, bool& cmdEchoed) {
    FT_STATUS status;
    uint32_t bytesInInputBuf = 0;
    uint32_t numOfBytesRead = 0;
    uint32_t bytesWritten;

    uint32_t loopCounter = 0;

    uchar_t readBuffer[MID_MAX_IN_BUF_SIZE];
    /*initialize cmdEchoed to MID_CMD_NOT_ECHOED*/
    cmdEchoed = false;
    /* check whether command has to be sent only once*/
    if(echoCmdFlag == false) {
        status = FT_Write(mHandle, &ecoCmd, 1, &bytesWritten);
        handleStatus(status);
    }
    do {
        /*check whether command has to be sent every time in the loop*/
        if(echoCmdFlag == true) {
            status = FT_Write(mHandle, &ecoCmd, 1, &bytesWritten);
            handleStatus(status);
        }
        /*read the no of bytes available in Receive buffer*/
        status = FT_GetQueueStatus(mHandle, &bytesInInputBuf);
        handleStatus(status);
        sleep(1);
        if(bytesInInputBuf > 0) {
            if(bytesInInputBuf > MID_MAX_IN_BUF_SIZE)
                handleStatus(status);

            status = FT_Read(mHandle, readBuffer, bytesInInputBuf, &numOfBytesRead);
            handleStatus(status);
            if(numOfBytesRead) {
                uint32_t byteCounter = 0;
                uchar_t cmdResponse = 0;
                do {
                    if(byteCounter <= (numOfBytesRead - 1)) {
                        if(readBuffer[byteCounter] == MID_BAD_COMMAND_RESPONSE)
                            cmdResponse = MID_BAD_COMMAND_RESPONSE;
                        else {
                            if(cmdResponse == MID_BAD_COMMAND_RESPONSE)
                                if(readBuffer[byteCounter] == ecoCmd)
                                    cmdEchoed = true;
                            cmdResponse = 0;
                        }
                    }
                    byteCounter++;
                } while(byteCounter < bytesInInputBuf && cmdEchoed == false);
            }
        }

        /*for breaking the loop */
        loopCounter++;
        if(loopCounter > MID_MAX_IN_BUF_SIZE)
            throw runtime_error(decodeStatus(FT_OTHER_ERROR));
    } while(cmdEchoed == false);
}

void Module::sleep(size_t time) {
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

}
