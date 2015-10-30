#include "caenv2718.hpp"

#include <CAENVME/CAENVMElib.h>

#include <chrono>

using std::string;
using std::vector;
using std::runtime_error;

using std::chrono::milliseconds;
using std::chrono::system_clock;

enum class CaenV2718::Reg : uint16_t {
		outputBuffer	= 0x0000,
		controlReg		= 0x1000,
		statusReg		= 0x1002,
		softwareReset   = 0x1014,
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

enum class CaenV2718::OpCode : uint16_t {
	setTrigMode     = 0x0000,
	setContMode	    = 0x0100,
	getMode			= 0x0200,
	setWinWidth		= 0x1000,
	setWinOffset	= 0x1100,
	enableTrigSub	= 0x1400,
	disableTrigSub	= 0x1500,
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

class CaenV2718::Decoder {
public:
	static void decode(unsigned lsb, const uint32_t* data, size_t size,
		               vector<Tdc::EventHits>& buffer) {
		Tdc::EventHits tmp;
		bool header = false;
		for(size_t i = 0; i < size; ++i) {
			if(isMeasurement(data[i]))
				tmp.push_back({ chan(data[i]), lsb*time(data[i]) });
			else if(isGlobalHeader(data[i]) && !header)
				header = true;
			else if(isGlobalTrailer(data[i]) && header) {
				header = false;
				buffer.push_back(tmp);
				tmp.clear();
			}
		}
	}
protected:
	static uint32_t time(uint32_t data) { return (data & TDC_MSR_MEASURE_MSK);}
	static uint32_t chan(uint32_t data) { return (data & TDC_MSR_CHANNEL_MSK) >> 19;}
	static bool isGlobalHeader(uint32_t data) { return (data & DATA_TYPE_MSK) == HEADER;}
	static bool isGlobalTrailer(uint32_t data) {return (data & DATA_TYPE_MSK) == TRAILER;}
	static bool isMeasurement(uint32_t data) {return (data & DATA_TYPE_MSK) == TDC_MEASURE;}
private:
	static const uint32_t DATA_TYPE_MSK = 0xf8000000; /* Data type bit masks */
	static const uint32_t HEADER = 0x40000000;      /* Global header data type */
	static const uint32_t TRAILER = 0x80000000;     /* Global trailer data type */
	static const uint32_t TDC_MEASURE = 0x00000000; /* TDC measure data type */
	static const uint32_t TDC_MSR_TRAILING_MSK = 0x04000000;
	static const uint32_t TDC_MSR_CHANNEL_MSK = 0x03f80000;
	static const uint32_t TDC_MSR_MEASURE_MSK = 0x0007ffff;
};

CaenV2718::CaenV2718(unsigned baseAddress)
	: mBaseAddress(baseAddress),
	  mIsInit(false) { }

CaenV2718::~CaenV2718() {
	close();
}

void CaenV2718::open() {
	if(mIsInit)
		throw runtime_error("CAENVME already init");
	auto status = CAENVME_Init(cvV2718, 0, 0, &mHandle);
	if(status != cvSuccess)
		throw runtime_error(CAENVME_DecodeError(status));
	mCtrl = ctrl();
	mLsb  = lsb();
	setTdcMeta(false);
	mIsInit = true;
}

void CaenV2718::close() {
	if(!mIsInit)
		return;
	CAENVME_End(mHandle);
	mIsInit = false;
}

bool CaenV2718::isOpen() const {
	return mIsInit;
}

CaenV2718::Settings CaenV2718::settings() {
	return {
		windowWidth(),
		windowOffset(),
		edgeDetection(),
		lsb()
	};
}

void CaenV2718::clear() {
	writeCycle16(Reg::softwareClear, 1);
}

void CaenV2718::reset() {
	writeCycle16(Reg::softwareReset, 1);
}

void CaenV2718::read(vector<EventHits>& buffer) {
	buffer.clear();
	static uint32_t buf[1024];
	int readBytes;
	auto errCode = CAENVME_BLTReadCycle(
		mHandle,
		formAddress(Reg::outputBuffer),
		buf, sizeof(buf),
		cvA32_U_BLT, cvD32,
		&readBytes
	);
	if((errCode == cvBusError && (mCtrl & 1)) || errCode == cvSuccess) {
		size_t readSize = size_t(readBytes / sizeof(uint32_t));
		Decoder::decode(mLsb, buf, readSize, buffer);
	} else {
		buffer.clear();
		throw runtime_error("CaenV2718::read: failed");
	}
}

const string& CaenV2718::name() const {
	static string n("CaenV2718");
	return n;
}

void CaenV2718::setMode(Mode mode) {
	switch(mode) {
	case Mode::trigger:
		return setTriggerMode();
	case Mode::continuous:
		return setContinuousMode();
	default:
		throw runtime_error(CAENVME_DecodeError(cvInvalidParam));
	}
}

void CaenV2718::setWindowWidth(unsigned width) {
	auto value = uint16_t(width / 25);
	if(value < 1 || value > 4095)
		throw runtime_error(CAENVME_DecodeError(cvInvalidParam));
	writeMicro(OpCode::setWinWidth, &value, 1);
}

void CaenV2718::setWindowOffset(int offset) {
	auto value = uint16_t(offset / 25);
	if(int16_t(value) < -2048 || int16_t(value) > 40)
		throw runtime_error(CAENVME_DecodeError(cvInvalidParam));
	writeMicro(OpCode::setWinOffset, &value, 1);
}

void CaenV2718::setEdgeDetection(EdgeDetection ed) {
	uint16_t value;
	switch(ed) {
	case EdgeDetection::trailing:
		value = 1;
		break;
	case EdgeDetection::leading:
		value = 2;
		break;
	default:
		throw runtime_error(CAENVME_DecodeError(cvInvalidParam));
	}
	writeMicro(OpCode::setDetection, &value, 1);
}
void CaenV2718::setLsb(unsigned ps) {
	uint16_t value;
	switch(ps) {
	case 100:
		value = 2;
		break;
	case 200:
		value = 1;
	case 800:
		value = 0;
	default:
		throw runtime_error(CAENVME_DecodeError(cvInvalidParam));
	}
	writeMicro(OpCode::setDetection, &value, 1);
	mLsb = value;
}

CaenV2718::Mode CaenV2718::mode() {
	uint16_t value;
	readMicro(OpCode::getMode, &value, 1);
	switch(value) {
	case 0:
		return Mode::continuous;
	case 1:
		return Mode::trigger;
	default:
		throw runtime_error("CaenV2718::mode unknown mode");
	}
}

unsigned CaenV2718::windowWidth() {
	return getTriggerConf().at(0) * 25;
}

int CaenV2718::windowOffset() {
	return getTriggerConf().at(1) * 25;
}

CaenV2718::EdgeDetection CaenV2718::edgeDetection() {
	uint16_t value;
	readMicro(OpCode::getDetection, &value, 1);
	switch(value) {
	case 1:
		return EdgeDetection::trailing;
	case 2:
		return EdgeDetection::leading;
	default:
		throw runtime_error("CaenV2718::lsb unknown lsb");
	}
}

unsigned CaenV2718::lsb() {
	uint16_t value;
	readMicro(OpCode::getLSB, &value, 1);
	switch(value) {
	case 2:
		mLsb = 100;
		break;
	case 1:
		mLsb = 200;
		break;
	case 0:
		mLsb = 800;
		break;
	default:
		throw runtime_error("CaenV2718::lsb unknown lsb");
	}
	return mLsb;
}

void CaenV2718::setTdcMeta(bool flag) {
	if(flag) {
		writeMicro(OpCode::enableTdcMeta, nullptr, 0);
	} else {
		writeMicro(OpCode::disableTdcMeta, nullptr, 0);
	}
}

void CaenV2718::setCtrl(uint16_t ctrl) {
	writeCycle16(Reg::controlReg, ctrl);
	mCtrl = ctrl;
}

uint16_t CaenV2718::ctrl() {
	mCtrl = readCycle16(Reg::controlReg);
	return mCtrl;
}

uint16_t CaenV2718::stat() {
	return readCycle16(Reg::statusReg);
}


void CaenV2718::setTriggerMode() {
	writeMicro(OpCode::setTrigMode, nullptr, 0);
	writeMicro(OpCode::enableTrigSub, nullptr, 0);
}

void CaenV2718::setContinuousMode() {
	writeMicro(OpCode::setContMode, nullptr, 0);
}

uint16_t CaenV2718::readCycle16(Reg addr) {
	uint16_t word;
	auto errCode = CAENVME_ReadCycle(
		mHandle,
		formAddress(addr),
		reinterpret_cast<void*>(&word),
		cvA32_S_DATA, cvD16
	);
	if(errCode != cvSuccess)
		throw runtime_error(CAENVME_DecodeError(errCode));
	return word;
}
void CaenV2718::writeCycle16(Reg addr, uint16_t word) {
	auto errCode = CAENVME_WriteCycle(
		mHandle,
		formAddress(addr),
		reinterpret_cast<void*>(&word),
		cvA32_S_DATA, cvD16
	);
	if(errCode != cvSuccess)
		throw runtime_error(CAENVME_DecodeError(errCode));
}

uint32_t CaenV2718::readCycle32(Reg addr) {
	uint16_t word;
	auto errCode = CAENVME_ReadCycle(
		mHandle,
		formAddress(addr),
		reinterpret_cast<void*>(&word),
		cvA32_S_DATA, cvD32
	);
	if(errCode != cvSuccess)
		throw runtime_error(CAENVME_DecodeError(errCode));
	return word;
}
void CaenV2718::writeCycle32(Reg addr, uint32_t word) {
	auto errCode = CAENVME_WriteCycle(
		mHandle,
		formAddress(addr),
		reinterpret_cast<void*>(&word),
		cvA32_S_DATA, cvD32
	);
	if(errCode != cvSuccess)
		throw runtime_error(CAENVME_DecodeError(errCode));
}

void CaenV2718::writeMicro(OpCode code, const uint16_t* data, size_t size) {
	checkMicroWrite();
	//Указываем opCode
	writeCycle16(Reg::micro, uint16_t(code));
	if(size == 0)
		return;
	do {
		//Проверяем handshake
		checkMicroWrite();
		writeCycle16(Reg::micro, *data);
		++data;
	} while(--size);
}

void CaenV2718::readMicro(OpCode code, uint16_t* data, size_t size) {
	checkMicroWrite();
	//Указываем opCode
	writeCycle16(Reg::micro, uint16_t(code));
	if(size == 0)
		return;
	do {
		checkMicroRead();
		*data = readCycle16(Reg::micro);
		++data;
	} while(--size);
}

CaenV2718::TriggerConf CaenV2718::getTriggerConf() {
	TriggerConf conf;
	readMicro(OpCode::getTrigConf, conf.data(), conf.size());
	return conf;
}

void CaenV2718::checkMicroWrite() {
	static constexpr uint16_t hShakeWo = 1;

	milliseconds timeout(500);
	auto s = system_clock::now();
	while(system_clock::now() - s < timeout) {
		auto hShake = readCycle16(Reg::handshake);
		if(hShake & hShakeWo)
			return;
	}
	throw runtime_error(CAENVME_DecodeError(cvTimeoutError));
}

void CaenV2718::checkMicroRead() {
	static constexpr uint16_t hShakeRo = 2;

	milliseconds timeout(500);
	auto s = system_clock::now();
	while(system_clock::now() - s < timeout) {
		auto hShake = readCycle16(Reg::handshake);
		if(hShake & hShakeRo)
			return;
	}
	throw runtime_error(CAENVME_DecodeError(cvTimeoutError));
}

uint32_t CaenV2718::formAddress(Reg addr) const {
	return ((uint32_t(mBaseAddress)) << 16) | uint16_t(addr);
}
