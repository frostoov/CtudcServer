#pragma once

#include "tdc.hpp"

#include "emiss/controlerem1.hpp"
#include "emiss/controlerem8.hpp"
//base - 0170000
class EmissTdc : public Tdc {
public:
    EmissTdc();
    void open();
    void close();
    
    void readEvents(std::vector<EventHits>& buffer) override;
    void readHits(std::vector<Hit>& buffer) override;
    const std::string& name() const override;
    Settings settings() override;
    bool isOpen() const override;
    void clear() override;
    Mode mode() override;
    void setMode(Mode mode) override;
private:
    ContrEM1 mEM1;
    ContrEM8 mEM8;
    std::vector<uint32_t> mBuffer;
};
