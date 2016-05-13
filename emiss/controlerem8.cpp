#include "controlerem8.hpp"

#include <stdexcept>
#include <iostream>
#include <limits>

using std::runtime_error;
using std::logic_error;

ContrEM8::ContrEM8(const Conf& conf)
    : mHandle(nullptr),
      mConf(conf) { }

ContrEM8::~ContrEM8() {
    if(mHandle != nullptr)
        close();
}

void ContrEM8::open(uint16_t vid, uint16_t pid) {
    if(mHandle != nullptr)
        throw logic_error("ContrEM8::open device is opened");
    mHandle = libusb_open_device_with_vid_pid(nullptr, vid, pid);
    if(mHandle == nullptr)
        throw runtime_error("ContrEM8::open device failed");
    
    auto status = libusb_kernel_driver_active(mHandle, mConf.interface);
    if(status < 0)
        throw runtime_error(libusb_strerror(libusb_error(status)));
    else if(status == 1) {
        auto status = libusb_detach_kernel_driver(mHandle, mConf.interface);
        if(status != 0)
            throw runtime_error(libusb_strerror(libusb_error(status)));
    }
    
    status = libusb_claim_interface(mHandle, mConf.interface);
    if(status != 0)
        throw runtime_error(libusb_strerror(libusb_error(status)));
}

void ContrEM8::close() {
    if(mHandle == nullptr)
        throw logic_error("ContrEM8::close device is closed");
    auto status = libusb_release_interface(mHandle, mConf.interface);
    if(status != 0)
        std::cerr << "libusb_release_interface " << libusb_strerror(libusb_error(status)) << std::endl;
    
    status = libusb_attach_kernel_driver(mHandle, mConf.interface);
    if(status != 0)
        std::cerr << "libusb_attach_kernel_driver " << libusb_strerror(libusb_error(status)) << std::endl;
    
    libusb_close(mHandle);
    mHandle = nullptr;
}

bool ContrEM8::isOpen() const {
    return mHandle != nullptr;
}

size_t ContrEM8::readData(std::vector<uint32_t>& buffer) {
	size_t length = buffer.size()*sizeof(uint32_t);
	if(length == 0 || length > size_t(std::numeric_limits<int>::max()))
		throw runtime_error("ContrEM8::readData invalid buffer size");
	auto ptr = reinterpret_cast<unsigned char*>(buffer.data());
	int transfered = 0;
	int status = libusb_bulk_transfer(
		mHandle,
		mConf.endpoint,
		ptr,
		int(length),
		&transfered,
		mConf.timeout);
	if(status != 0)
		throw runtime_error(libusb_strerror(libusb_error(status)));
	if(transfered % sizeof(uint32_t) != 0)
		throw runtime_error("ContrEM8::readData invalid transfer size");
	return size_t( transfered/sizeof(uint32_t) );
}
