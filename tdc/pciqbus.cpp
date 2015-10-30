#include "pciqbus.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>


using std::runtime_error;

PciQbus::PciQbus()
    : mNode(-1), mCurrentAddr(-1) { }

PciQbus::PciQbus(const std::string& deviceName)
    : mNode(-1),
      mCurrentAddr(-1) { this->open(deviceName); }

PciQbus::~PciQbus() { this->close(); }

void PciQbus::open(const std::string& deviceName) {
    if(isOpen())
        throw runtime_error("PciQbus::open device is already opened");
    mNode = ::open(deviceName.data(), O_RDWR);
    if(mNode == -1)
        throw runtime_error("PciQbus::open falied");
    resetBranch();
    clearErrorStatus();
}

void PciQbus::close() {
    if(::close(mNode) == -1)
        throw runtime_error("PciQbus::close failed");
    mNode = -1;
}

bool PciQbus::isOpen() const {
    return mNode != -1;
}

void PciQbus::clearErrorStatus() {
    if(::ioctl(mNode, 0) != 0)
        throw runtime_error("PciQbus::clearErrorStatus failed");
}

void PciQbus::resetBranch() {
    if(::ioctl(mNode, 1) != 0)
        throw runtime_error("PciQbus::resetBranch failed");
}

size_t PciQbus::read(long addr, uint16_t* data, size_t size) {
    this->seek(addr);
    ssize_t count = ::read(mNode, data, size*sizeof(uint16_t));
    if(count == -1)
        throw runtime_error("PciQbus::read failed");
    return count;
}

size_t PciQbus::write(long addr, const uint16_t* data, size_t size) {
    this->seek(addr);
    ssize_t count = ::write(mNode, data, size*sizeof(uint16_t));
    if(count == -1)
        throw runtime_error("PciQbus::read failed");
    return count;
}

uint16_t PciQbus::readWord(long addr) {
    uint16_t word;
    this->seek(addr);
    if(::read(mNode, &word, sizeof(word)) != sizeof(word))
        throw runtime_error("PciQbus::readWord failed");
    return word;
}

void PciQbus::writeWord(long addr, uint16_t word) {
    this->seek(addr);
    if( ::write(mNode, &word, sizeof(word)) != sizeof(word))
        throw runtime_error("PciQbus::writeWord failed");
}

void PciQbus::seek(long addr) {
    if(addr != mCurrentAddr) {
        if(::lseek(mNode, addr, SEEK_SET) == -1)
            throw runtime_error("PciQbus::seek failed");
        mCurrentAddr = addr;
    }
}
