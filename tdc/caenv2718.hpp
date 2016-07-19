#pragma once

#include "tdc.hpp"

#include <array>
#include <mutex>

class CaenV2718 : public Tdc {
    enum class Reg : uint16_t;
    enum class OpCode : uint16_t;
    using Mutex = std::mutex;
    using Lock = std::lock_guard<Mutex>;
    using TriggerConf = std::array<uint16_t, 5>;
public:
    CaenV2718(const std::string& name, unsigned vmeAddress);
    ~CaenV2718();

    void readEvents(std::vector<EventHits>& buffer) override;
    void readHits(std::vector<Hit>& buffer) override;
    const std::string& name() const override;
    Settings settings() override;
    bool isOpen() const override;
    void clear() override;

    void open();
    void close();
    void updateSettings();

    void reset();

    Mode mode() override;
    void setMode(Mode mode) override;

    unsigned windowWidth();
    void setWindowWidth(unsigned width);

    int windowOffset();
    void setWindowOffset(int offset);

    EdgeDetection edgeDetection();
    void setEdgeDetection(EdgeDetection ed);

    unsigned lsb();
    void setLsb(unsigned ps);

    void setTdcMeta(bool flag);
    bool tdcMeta();

    void setCtrl(uint16_t ctrl);
    uint16_t ctrl();

    uint16_t stat();
protected:
    template<typename B, typename D>
    void readData(B& buffer, D& decoder);
    void setTriggerMode();
    void setContinuousMode();

    uint16_t readCycle16(Reg addr);
    void writeCycle16(Reg addr, uint16_t word);

    uint32_t readCycle32(Reg addr);
    void writeCycle32(Reg addr, uint32_t);

    void writeMicro(OpCode code, const uint16_t* data, size_t size);
    void readMicro(OpCode code, uint16_t* data, size_t size);

    void checkMicroWrite();
    void checkMicroRead();

    TriggerConf getTriggerConf();
    inline uint32_t formAddress(Reg addr) const;
private:
    int32_t mHandle;
    uint32_t mBaseAddress;
    bool mIsInit;
    Mutex mMutex;
    Mutex mMicroMutex;

    mutable Settings mSettings;
    mutable uint16_t mCtrl;

    std::string mName;
};
