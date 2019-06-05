#include "pciqbus.hpp"

#include <trek/common/stringbuilder.hpp>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <cerrno>
#include <cstring>

using trek::StringBuilder;

using std::vector;
using std::strerror;
using std::runtime_error;

PciQbus::PciQbus()
    : mNode(-1), mCurrentAddr(-1) { }

PciQbus::PciQbus(const std::string& deviceName)
    : mNode(-1),
      mCurrentAddr(-1) { this->open(deviceName); }

PciQbus::~PciQbus() { this->close(); }

void PciQbus::open(const std::string& deviceName) {
    if(mNode != -1)
        error("open", "device already opened");
    mNode = ::open(deviceName.data(), O_RDWR);
    if(mNode < 0)
        error("open", strerror(errno));
}

void PciQbus::close() {
    ::close(mNode);
    mNode = -1;
}

bool PciQbus::isOpen() const {
    return mNode != -1;
}

void PciQbus::clearErrorStatus() {
    if(::ioctl(mNode, 0) != 0)
        error("clearErrorStatus", strerror(errno));
}

void PciQbus::resetBranch() {
    if(::ioctl(mNode, 1) != 0)
        error("resetBranch", strerror(errno));
}

size_t PciQbus::read(long addr, vector<uint16_t>& buffer) {
    this->seek(addr);
    ssize_t count = ::read(mNode, buffer.data(), buffer.size()*sizeof(uint16_t));
    if(count == -1)
        error("read", "failed");
    return count;
}

size_t PciQbus::write(long addr, const vector<uint16_t>& buffer) {
    this->seek(addr);
    ssize_t count = ::write(mNode, buffer.data(), buffer.size()*sizeof(uint16_t));
    if(count == -1)
        error("write", "failed");
    return count;
}

uint16_t PciQbus::readWord(long addr) {
    static uint16_t word;
    this->seek(addr);
    auto size = ::read(mNode, &word, sizeof(word));
    //if(::read(mNode, &word, sizeof(word)) != sizeof(word))
    if(size != sizeof(word))
        error("readWord", "failed");
    return word;
}

void PciQbus::writeWord(long addr, uint16_t word) {
    this->seek(addr);
    if( ::write(mNode, &word, sizeof(word)) != sizeof(word))
        error("writeWord", "failed");
}

void PciQbus::seek(long addr) {
    if(addr != mCurrentAddr) {
        if(::lseek(mNode, addr, SEEK_SET) == -1)
            error("seek", strerror(errno));
        mCurrentAddr = addr;
    }
}

void PciQbus::error(const std::string& operation, const char* err) {
    throw runtime_error(StringBuilder() << "QBus::" << operation << ": " << err);
}
