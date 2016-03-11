#include "caenv2718.hpp"

#include <CAENVMElib.h>

#include <chrono>
#include <thread>

using std::string;
using std::vector;
using std::runtime_error;
using std::logic_error;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

enum class CaenV2718::Reg : uint16_t {
	outputBuffer  = 0x0000,
	controlReg    = 0x1000,
	statusReg     = 0x1002,
	softwareReset = 0x1014,
	softwareClear = 0x1016,
	eventReset    = 0x1018,
	eventBLT      = 0x1024,
	eventCounter  = 0x101C,
	eventStored   = 0x1020,
	almostFull    = 0x1022,
	firmwareRev   = 0x1026,
	micro         = 0x102E,
	handshake     = 0x1030
};

enum class CaenV2718::OpCode : uint16_t {
	setTrigMode    = 0x0000,
	setContMode    = 0x0100,
	getMode        = 0x0200,
	setWinWidth    = 0x1000,
	setWinOffset   = 0x1100,
	enableTrigSub  = 0x1400,
	disableTrigSub = 0x1500,
	getTrigConf    = 0x1600,
	setDetection   = 0x2200,
	getDetection   = 0x2300,
	setLSB         = 0x2400,
	getLSB         = 0x2600,
	setDeadTime    = 0x2800,
	getDeadTime    = 0x2900,
	enableTdcMeta  = 0x3000,
	disableTdcMeta = 0x3100,
	getTdcMeta     = 0x3200,
	microRev       = 0x6100
};

class CaenV2718::Decoder {
public:
	static void decode(const CaenV2718::Settings& settings,
		               const uint32_t* data, size_t size,
	                   vector<Tdc::EventHits>& buffer) {
		buffer.clear();
		bool header = false;
		for(size_t i = 0; i < size; ++i) {
			if(isMeasurement(data[i]) && !buffer.empty() )
				buffer.back().emplace_back(edgeDetection(data[i]), chan(data[i]), settings.lsb * time(data[i]));
			else if(isGlobalHeader(data[i]) && !header) {
				buffer.emplace_back();
				header = true;
			} else if(isGlobalTrailer(data[i]) && header)
				header = false;
		}
	}
	static void decodeRaw(const CaenV2718::Settings& settings,
		                  const uint32_t* data, size_t size,
						  vector<Tdc::Hit>& buffer) {
		buffer.clear();
		for(size_t i = 0; i < size; ++i)
			if(isMeasurement(data[i]) )
				buffer.emplace_back(edgeDetection(data[i]), chan(data[i]), settings.lsb * time(data[i]));
	}
protected:
	static unsigned time(uint32_t data) { return (data & TDC_MSR_MEASURE_MSK);}
	static unsigned chan(uint32_t data) { return (data & TDC_MSR_CHANNEL_MSK) >> 19;}
	static bool isGlobalHeader(uint32_t data) { return (data & DATA_TYPE_MSK) == HEADER;}
	static bool isGlobalTrailer(uint32_t data) {return (data & DATA_TYPE_MSK) == TRAILER;}
	static bool isMeasurement(uint32_t data) {return (data & DATA_TYPE_MSK) == TDC_MEASURE;}
	static EdgeDetection edgeDetection(uint32_t data) {
		if((data >> 26) > 0)
			return Tdc::EdgeDetection::trailing;
		return Tdc::EdgeDetection::leading;
	}
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
		throw logic_error("CAENVME already init");
	auto status = CAENVME_Init(cvV2718, 0, 0, &mHandle);
	if(status != cvSuccess)
		throw runtime_error(CAENVME_DecodeError(status));
	mIsInit = true;
	std::this_thread::sleep_for(milliseconds(150));
	updateSettings();
	mCtrl = ctrl();
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
	if(!mIsInit)
		throw logic_error("CaenV2718::settings device is not open");
	return mSettings;
}

void CaenV2718::clear() {
	writeCycle16(Reg::softwareClear, 1);
}

void CaenV2718::reset() {
	writeCycle16(Reg::softwareReset, 1);
}

void CaenV2718::readEvents(vector<EventHits>& buffer) {
	if(!mIsInit)
		throw logic_error("CaenV2718::read device is not open");
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
		Decoder::decode(mSettings, buf, readSize, buffer);
	} else {
		buffer.clear();
		throw runtime_error("CaenV2718::read: failed");
	}
}

void CaenV2718::readRaw(vector<Hit>& buffer) {
	if(!mIsInit)
		throw logic_error("CaenV2718::read device is not open");
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
		Decoder::decodeRaw(mSettings, buf, readSize, buffer);
	} else {
		buffer.clear();
		throw runtime_error("CaenV2718::read: failed");
	}
}

const string& CaenV2718::name() const {
	static string n("CaenV2718");
	return n;
}

void CaenV2718::updateSettings() {
	mSettings = {
		windowWidth(),
		windowOffset(),
		edgeDetection(),
		lsb(),
	};
}

void CaenV2718::setMode(Mode mode) {
	switch(mode) {
	case Mode::trigger:
		return setTriggerMode();
	case Mode::continuous:
		return setContinuousMode();
	default:
		throw logic_error(CAENVME_DecodeError(cvInvalidParam));
	}
}

void CaenV2718::setWindowWidth(unsigned width) {
	auto value = uint16_t(width / 25);
	if(value < 1 || value > 4095)
		throw logic_error(CAENVME_DecodeError(cvInvalidParam));
	writeMicro(OpCode::setWinWidth, &value, 1);
	mSettings.windowWidth = (width/25)*25;
}

void CaenV2718::setWindowOffset(int offset) {
	auto value = uint16_t(offset / 25);
	if(int16_t(value) < -2048 || int16_t(value) > 40)
		throw logic_error(CAENVME_DecodeError(cvInvalidParam));
	writeMicro(OpCode::setWinOffset, &value, 1);
	mSettings.windowOffset = (offset/25)*25;
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
	case EdgeDetection::leadingTrailing:
		value = 3;
		break;
	default:
		throw logic_error(CAENVME_DecodeError(cvInvalidParam));
	}
	writeMicro(OpCode::setDetection, &value, 1);
	mSettings.edgeDetection = ed;
}
void CaenV2718::setLsb(unsigned ps) {
	uint16_t value;
	switch(ps) {
	case 98:
		value = 2;
		break;
	case 195:
		value = 1;
		break;
	case 781:
		value = 0;
		break;
	default:
		throw logic_error(CAENVME_DecodeError(cvInvalidParam));
	}
	writeMicro(OpCode::setLSB, &value, 1);
	mSettings.lsb = value;
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
	return int16_t(getTriggerConf().at(1)) * 25;
}

CaenV2718::EdgeDetection CaenV2718::edgeDetection() {
	uint16_t value;
	readMicro(OpCode::getDetection, &value, 1);
	switch(value) {
	case 1:
		return EdgeDetection::trailing;
	case 2:
		return EdgeDetection::leading;
	case 3:
		return EdgeDetection::leadingTrailing;
	default:
		throw runtime_error("CaenV2718::edgeDetection unknown edgeDetection");
	}
}

unsigned CaenV2718::lsb() {
	uint16_t value;
	readMicro(OpCode::getLSB, &value, 1);
	switch(value) {
	case 2:
		mSettings.lsb = 98;
		break;
	case 1:
		mSettings.lsb = 195;
		break;
	case 0:
		mSettings.lsb = 781;
		break;
	default:
		throw runtime_error("CaenV2718::lsb unknown lsb");
	}
	return mSettings.lsb;
}

void CaenV2718::setTdcMeta(bool flag) {
	if(flag) {
		writeMicro(OpCode::enableTdcMeta, nullptr, 0);
	} else {
		writeMicro(OpCode::disableTdcMeta, nullptr, 0);
	}
}

bool CaenV2718::tdcMeta() {
	uint16_t word;
	readMicro(OpCode::getTdcMeta, &word, 1);
	return word != 0;
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
	if(!mIsInit)
		throw logic_error("CaenV2718::readCycle15 device is not open");
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
	if(!mIsInit)
		throw logic_error("CaenV2718::readCycle15 device is not open");
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
	if(!mIsInit)
		throw logic_error("CaenV2718::readCycle15 device is not open");
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
	if(!mIsInit)
		throw logic_error("CaenV2718::readCycle15 device is not open");
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

	seconds timeout(2);
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

	seconds timeout(2);
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
