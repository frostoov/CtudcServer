#ifndef MODULEMANAGER_HPP
#define MODULEMANAGER_HPP

#include <thread>
#include <list>
#include <memory>

#include <trekdata/tdcrecord.hpp>

#include "threadmanager.hpp"
#include "caen/tdcmodule.hpp"
#include "channelconfig.hpp"

namespace caen {

class ProcessManager : public ThreadManager {
protected:
    using ModulePtr     = std::shared_ptr<Module>;
    using TdcRecordList  = std::list<trekdata::TdcRecord>;
public:

    void setModule(ModulePtr module) {
        mTdcModule = module;
    }

    static uint32_t GLBHeaderEventCount(uint32_t data) { return (data & GLB_HDR_EVENT_COUNT_MSK) >> 5; }
    static uint32_t GLBHeaderGeo(uint32_t data)        { return (data & GLB_HDR_GEO_MSK);}
    static uint32_t GLBTrigTimeTag(uint32_t data)      { return (data & GLB_TRG_TIME_TAG_MSK); }
    static uint32_t GLBTrailerStat(uint32_t data)      { return (data & GLB_TRL_STATUS_MSK) >> 24; }
    static uint32_t GLBTrailerWCount(uint32_t data)    { return (data & GLB_TRL_WCOUNT_MSK) >> 5; }
    static uint32_t GLBTrailerGeo(uint32_t data)       { return (data & GLB_TRL_GEO_MSK); }
    static uint32_t TDCHeaderTdc(uint32_t data)        { return (data & TDC_HDR_TDC_MSK) >> 24; }
    static uint32_t TDCHeaderEvtID(uint32_t data)      { return (data & TDC_HDR_EVENT_ID_MSK) >> 12; }
    static uint32_t TDCHeaderBunchID(uint32_t data)    { return (data & TDC_HDR_BUNCH_ID_MSK); }
    static uint32_t TDCMsrTrailing(uint32_t data)      { return (data & TDC_MSR_TRAILING_MSK) >> 26; }
    static uint32_t TDCMsrMeasure(uint32_t data)       { return (data & TDC_MSR_MEASURE_MSK); }
    static uint32_t TDCTrailerTdc(uint32_t data)       { return (data & TDC_TRL_TDC_MSK) >> 24; }
    static uint32_t TDCTrailerEvtID(uint32_t data)     { return (data & TDC_TRL_EVENT_ID_MSK) >> 12; }
    static uint32_t TDCTrailerWCount(uint32_t data)    { return (data & TDC_TRL_WCOUNT_MSK); }
    static uint32_t TDCErrTdc(uint32_t data)           { return (data & TDC_ERR_TDC_MSK) >> 24; }
    static uint32_t TDCErrFlags(uint32_t data)         { return (data & TDC_ERR_ERR_FLAGS_MSK); }
    static uint32_t getChannel(uint32_t data)          { return (data & TDC_MSR_CHANNEL_MSK) >> 19; }
    static uint32_t getChamber(uint32_t data)          { return ((data >> 19) & 31); }
    static uint32_t getWire(uint32_t data)             { return ((data >> 24) & 3); }
    static bool isGlobalHeader(uint32_t data)          { return (data & DATA_TYPE_MSK) == HEADER; }
    static bool isGlobalTrailer(uint32_t data)         { return (data & DATA_TYPE_MSK) == TRAILER; }
    static bool isMeasurement(uint32_t data)           { return (data & DATA_TYPE_MSK) == TDC_MEASURE; }
protected:
    ProcessManager(ModulePtr module, const ChannelConfig& config);
    bool init() override;
    void setBkpSettings(const trekdata::Settings& settings);
    void returnSettings();

    TdcRecordList handleBuffer(const WordVector& buffer);
    uint32_t    convertWord(uint32_t word);
    WordVector& convertEvent(WordVector& eventWords);

    bool checkChannel(uint32_t channel) const;
    const ChannelCongruence& getChannelCongruence(size_t ch);
    ModulePtr mTdcModule;
private:
    ChannelConfig mConfig;

    trekdata::Settings mBkpSettings;

    static const uint32_t DATA_TYPE_MSK = 0xf8000000; /* Data type bit masks */

    static const uint32_t HEADER = 0x40000000;      /* Global header data type */
    static const uint32_t TRAILER = 0x80000000;     /* Global trailer data type */
    static const uint32_t TDC_HEADER = 0x08000000;  /* TDC header data type */
    static const uint32_t TDC_MEASURE = 0x00000000; /* TDC measure data type */
    static const uint32_t TDC_ERROR = 0x20000000;   /* TDC error data type */
    static const uint32_t TDC_TRAILER = 0x18000000; /* TDC trailer data type */
    static const uint32_t TRIGGER_TIME = 0x88000000;
    static const uint32_t FILLER = 0xc0000000;
    static const uint32_t GLB_HDR_EVENT_COUNT_MSK = 0x07ffffe0;
    static const uint32_t GLB_HDR_GEO_MSK = 0x0000001f;
    static const uint32_t GLB_TRG_TIME_TAG_MSK = 0x07ffffff;
    static const uint32_t GLB_TRL_STATUS_MSK = 0x07000000;
    static const uint32_t GLB_TRL_WCOUNT_MSK = 0x001fffe0;
    static const uint32_t GLB_TRL_GEO_MSK = 0x0000001f;

    static const uint32_t TDC_HDR_TDC_MSK = 0x03000000;
    static const uint32_t TDC_HDR_EVENT_ID_MSK = 0x00fff000;
    static const uint32_t TDC_HDR_BUNCH_ID_MSK = 0x00000fff;
    static const uint32_t TDC_MSR_TRAILING_MSK = 0x04000000;
    static const uint32_t TDC_MSR_CHANNEL_MSK = 0x03f80000;
    static const uint32_t TDC_MSR_MEASURE_MSK = 0x0007ffff;
    static const uint32_t TDC_TRL_TDC_MSK = 0x03000000;
    static const uint32_t TDC_TRL_EVENT_ID_MSK = 0x00fff000;
    static const uint32_t TDC_TRL_WCOUNT_MSK = 0x00000fff;
    static const uint32_t TDC_ERR_TDC_MSK = 0x03000000;
    static const uint32_t TDC_ERR_ERR_FLAGS_MSK = 0x00003fff;
};

} //caen

#endif  // MODULEMANAGER_HPP
