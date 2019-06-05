/*
 * author: Plotnikov I.S.
 * e-mail: Ivan.Plotnikov@ihep.ru
 */

#pragma once

#include "pciqbus.hpp"

#include <unordered_set>
#include <vector>

class ContrEM1 {
public:
    enum class TypeSignal : uint16_t {
        pulse = 0, potential = 1,
   };
public:
    ContrEM1(long baseAddress);
    
    void open(const std::string& devName);
    void close();
    bool isOpen() const;
    
    void resetQbus();
    void clearError();

    // запись в статусный регистр
    void writeStatus (uint16_t data);
    // чтение статусного регистра
    uint16_t readStatus();
    // состояние линии Гт магистрали сектора
    bool stateOfGt();                 
    void totalReset();

    void writeControl(uint16_t code);
    uint16_t readControl();

    void writeInterrupt(uint16_t code);
    uint16_t readInterrupt();

    // работа с евромисс
    void write(uint16_t nmModul, uint16_t addr, uint16_t data);
    uint16_t read(uint16_t nmModul, uint16_t addr);

    // чтение статусного слова режима ПЧ
    uint16_t readStatusPch();
    // чтение адресной памяти режима ПЧ
    uint16_t readAddressPch();
    // чтение памяти данных режима ПЧИ
    uint16_t readDataPch();                   

    size_t readDataPch(std::vector<uint16_t>& data);

    // установка режима работы сектора
    void setPCHI();                     // ПЧИ
    void setPCHN();                     // ПЧН
    void setAP();                       // АП
    void setAR();                       // АР

    // чтение номеров модулей установленных в каркасе
    std::unordered_set<int> readNumberPch();

    // сброс сигнала       (if NIM - High potential)
    void resetSignal(uint16_t nmOut);
    // генерация сигнала, заданного типа
    void generateSignal(TypeSignal sType, uint16_t nmOut );  
protected:
    void setTypeSignal(uint16_t codeOutSignal);
    void generateSignal(uint16_t nmOut);

    size_t readBlock(long addr, std::vector<uint16_t>& buffer) { 
        return mQbusDev.read(mBaseAddress + addr, buffer); 
    }
    size_t writeBlock(long addr, const std::vector< uint16_t>& buffer) { 
        return mQbusDev.write(mBaseAddress + addr, buffer); 
    }

    uint16_t readWord(long addr) { 
        return mQbusDev.readWord(mBaseAddress + addr); 
    }
    void writeWord(long addr, uint16_t data) { 
        return mQbusDev.writeWord(mBaseAddress + addr, data); 
    }
    // запись адреса в адр. рег. для операции записи
    void writeAddressToWrite(uint16_t data);
    // запись адреса в адр. рег. для операции чтения
    void writeAddressToRead(uint16_t data);
    // чтение адресного регистра
    uint16_t readAddressToWrite();                 
    uint16_t readAddressToRead();
    // запись в регистр данных
    void writeData(uint16_t data);
    // чтение регистра данных
    uint16_t readData();                
private:
    long mBaseAddress;// базовый адрес А0
    uint8_t mControlReg;
    PciQbus mQbusDev;
};

/*
 * АП  - адресная передача - режим, при котором для выполнения любой операции требуется занесение адреса в РА
 * АР  - режим, при котором модулями сектора управляет автономный контролер
 * ПЧН - последовательное чтение номеров модулей в крейте
 * ПЧИ - последовательное чтение информации из модулей
 */
