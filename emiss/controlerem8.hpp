#pragma once

#include <libusb.h>

#include <vector>

class ContrEM8 {
public:
    struct Conf {
        unsigned char endpoint;
        unsigned int timeout;
        int interface;
    };
public:
    ContrEM8(const Conf& conf);
    void open(uint16_t vid, uint16_t pid);
    void close();
    bool isOpen() const;
    ~ContrEM8();
    size_t readData(std::vector<uint32_t>& buffer);
private:
    libusb_device_handle* mHandle;
    Conf mConf;
};
