#pragma once

#include <stdexcept>
#include <cstdint>
#include <vector>

class PciQbus {
    using RunTimeError = std::runtime_error;
public:
    PciQbus();
    PciQbus(const std::string &deviceName);
    ~PciQbus();

    void open(const std::string& deviceName);
    void close();
    bool isOpen() const;


    void clearErrorStatus();
    void resetBranch();

    size_t read(long addr, std::vector<uint16_t>& buffer);
    size_t write(long addr, const std::vector<uint16_t>& buffer);

    uint16_t readWord(long addr);
    void writeWord(long addr, uint16_t word);
protected:
    void seek(long addr);
    void error(const std::string& operation, const char* err);
private:
    int mNode;
    long mCurrentAddr;
};

