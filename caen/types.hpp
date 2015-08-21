#ifndef CAEN_TYPES_H
#define CAEN_TYPES_H

#include <cstdint>
#include <vector>
#include <array>
#include <cstring>
#include <typeinfo>
#include <CAENVME/CAENVMEtypes.h>
#include <unordered_map>

namespace caen {

using WordVector    = std::vector<uint32_t>;

enum class Reg : uint16_t {
    outputBuffer	= 0x0000,
    controlReg		= 0x1000,
    statusReg		= 0x1002,
    softwareClear	= 0x1016,
    eventReset		= 0x1018,
    eventBLT		= 0x1024,
    eventCounter	= 0x101C,
    eventStored		= 0x1020,
    almostFull		= 0x1022,
    firmwareRev		= 0x1026,
    micro			= 0x102E,
    handshake		= 0x1030
};

enum class OpCode : uint16_t {
    setTriggerMatching		= 0x0000,
    setContinuousStorage	= 0x0100,
    getMode			= 0x0200,
    setWinWidth		= 0x1000,
    setWinOffset	= 0x1100,
    enableSubTrig	= 0x1400,
    disableSubTrig	= 0x1500,
    getTrigConf		= 0x1600,
    setDetection	= 0x2200,
    getDetection	= 0x2300,
    setLSB			= 0x2400,
    getLSB			= 0x2600,
    setDeadTime		= 0x2800,
    getDeadTime		= 0x2900,
    enableTdcMeta	= 0x3000,
    disableTdcMeta	= 0x3100,
    readTdcMeta     = 0x3200,
    microRev		= 0x6100
};

}

#endif // CAEN_TYPES_H
