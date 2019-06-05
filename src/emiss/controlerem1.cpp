#include "controlerem1.hpp"

using std::vector;
using std::string;
using std::unordered_set;
using std::runtime_error;

ContrEM1::ContrEM1(long baseAddress)
    : mBaseAddress(baseAddress) { }

void ContrEM1::open(const string& devName) {
    mQbusDev.open(devName);
    mQbusDev.clearErrorStatus();
    mQbusDev.resetBranch();
    mControlReg = readControl();
}

void ContrEM1::close() {
    mQbusDev.close();
}

bool ContrEM1::isOpen() const {
    return mQbusDev.isOpen();
}

void ContrEM1::resetQbus() {
    mQbusDev.resetBranch();
}

void ContrEM1::clearError(){
    mQbusDev.clearErrorStatus();
}

void ContrEM1::writeStatus(uint16_t data) {
    writeWord(0, data);
}

uint16_t ContrEM1::readStatus() {
    return readWord(0);
}

void ContrEM1::totalReset() {
    writeStatus(0b1000000000000000);
}

void ContrEM1::writeData(uint16_t data) {
    writeWord(010, data);
}

uint16_t ContrEM1::readData() {
    return readWord(010);
}

void ContrEM1::writeAddressToWrite(uint16_t data) {
    writeWord(03, data);
}

void ContrEM1::writeAddressToRead(uint16_t data) {
    writeWord(02, data);
}

uint16_t ContrEM1::readAddressToWrite() {
    return readWord(03);
}

uint16_t ContrEM1::readAddressToRead() {
    return readWord(02);
}

void ContrEM1::writeControl(uint16_t code) {
    writeWord(06, code);
    mControlReg = code;
}

uint16_t ContrEM1::readControl() {
    return readWord(06);
}

void ContrEM1::writeInterrupt(uint16_t code) {
    writeWord(04, code);
}

uint16_t ContrEM1::readInterrupt() {
    return readWord(04);
}

void ContrEM1::write(uint16_t nmModul, uint16_t addr, uint16_t data) {
    uint16_t na = (addr << 5) | nmModul;
    writeData(data);
    writeAddressToWrite(na);
}

uint16_t ContrEM1::read(uint16_t nmModul, uint16_t addr) {
    uint16_t na = (addr << 5) | nmModul;
    writeAddressToRead(na);
    return readData();
}

uint16_t ContrEM1::readStatusPch() {
    return readWord(012);
}

uint16_t ContrEM1::readAddressPch() {
    return readWord(014);
}

uint16_t ContrEM1::readDataPch() {
    return this->readWord(016);
}

size_t ContrEM1::readDataPch(vector<uint16_t>& data) {
    return readBlock(016, data);
}

void ContrEM1::setPCHI() {
    mControlReg &= 0b00111111;
    mControlReg |= 0b10000000;
    writeControl(mControlReg);
}

void ContrEM1::setPCHN() {
    mControlReg &= 0b00111111;
    mControlReg |= 0b01000000;
    writeControl(mControlReg);
}

void ContrEM1::setAP() {
    mControlReg &= 0b00111111;
    mControlReg |= 0b00000000;
    writeControl(mControlReg);
}

void ContrEM1::setAR() {
    mControlReg &= 0b00111111;
    mControlReg |= 0b11000000;
    writeControl(mControlReg);
}

unordered_set<int> ContrEM1::readNumberPch() {
    bool f_ready;
    int pch_st;

    totalReset();
    setPCHN();
    do {
        pch_st = readStatusPch();
        f_ready = (pch_st & 040000) >> 14;
    } while(f_ready == 0);      // wait PCH READY bit

    int wq    =  pch_st & 03777;            // amount of words
    int errfl = (pch_st & 0100000) >> 15;   // error flags
    int ovf   = (pch_st & 020000) >> 13;    // переполнение буферной памяти
    // у флагов надо заменить int --> bool

    if(wq > 20)
        throw runtime_error("ContrEM1::readNumberPch too many modules\n");

    unordered_set<int> modules;
    for(int i = 0; i < wq; i++) {
        int mnum = readAddressPch() & 037;
        modules.insert(mnum);
    }
    return modules;
}

void ContrEM1::setTypeSignal(uint16_t codeOutSignal) {
    writeWord(011, codeOutSignal & 0x7);
}

void ContrEM1::generateSignal(uint16_t nmOut) {
    writeWord(012, nmOut & 0x3);
}


void ContrEM1::resetSignal(uint16_t nmOut) {
    writeWord(013, nmOut & 0x3);
}

void ContrEM1::generateSignal(TypeSignal sType, uint16_t nmOut) {
    setTypeSignal( uint16_t(sType) << nmOut );
    generateSignal(nmOut);
}
