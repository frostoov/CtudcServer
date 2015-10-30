#pragma once

#include <cstddef>
#include <string>

class PciQbus {
public:
    PciQbus();
    PciQbus(const std::string &deviceName);
    ~PciQbus();

    void open(const std::string& deviceName);
    void close();
    bool isOpen() const;

    void clearErrorStatus();
    void resetBranch();

    size_t read(long addr, uint16_t* data, size_t size);
    size_t write(long addr, const uint16_t* data, size_t size);

    uint16_t readWord(long addr);
    void writeWord(long addr, uint16_t word);
protected:
    void seek(long addr);
private:
	int mNode;
    long mCurrentAddr;
};
