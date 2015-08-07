#ifndef CAEN_MODULE_H
#define CAEN_MODULE_H

#include <chrono>
#include <functional>
#include <tdcdata/tdcsettings.hpp>

#include "observer/observer.hpp"
#include "types.hpp"

namespace caen {

class Module : public Subject {
	using Microseconds = std::chrono::microseconds;
	using Milliseconds = std::chrono::milliseconds;
  public:
	Module(const int32_t vmeAddress = 0);
	virtual ~Module();
	void setBaseAdress(int32_t address) { mBaseAddress = address; }
	bool isInit() const { return mIsInit; }
	const char* getTitle() const override {return "CAENVME TDC";}

	bool initialize();
	bool close();
	bool setSettings(const tdcdata::Settings& mSettings);
	bool setLsb(tdcdata::Lsb lsb);
	bool setWindowWidth(uint16_t windowWidth);
	bool setWindowOffset(int16_t winOffset);
	bool setAlmostFull(uint16_t value);
	bool setEdgeDetection(tdcdata::EdgeDetection edgeDetection);
	bool setControl(uint16_t control);
	bool setDeadTime(uint16_t deadTime);
	bool setEventBLT(uint16_t eventBLT);
	bool setTriggerMode(bool flag);
	bool setTriggerSubtraction(bool flag);
	bool setTdcMeta(bool flag);

	bool updateMode();
	bool updateTdcMeta();
	bool updateDetection();
	bool updateDeadTime();
	bool updateLSB();
	bool updateStatus();
	bool updateControl();
	bool updateAlmostFull();
	bool updateEventBLT();
	bool updateTriggerConfig();


	uint16_t getFirmwareRev();
	uint16_t getMicroRev();
	bool softwareClear();

	size_t readBlock(WordVector& buff, const Microseconds& delay = Microseconds::zero());
	size_t getBlockSize() const;

	bool updateSettings();
	const tdcdata::Settings& getSettings() const { return mSettings; }
  protected:
	uint32_t readReg32(Reg addr) const;
	uint16_t readReg16(Reg addr) const;
	void writeReg32(uint32_t data, Reg addr);
	void writeReg16(uint16_t data, Reg addr);

	void readMicro	(uint16_t* data, OpCode code, short num = 1);
	void writeMicro	(uint16_t* data, OpCode code, short num = 1);

	bool doAction(std::string&& message, std::function<void()>&& func);

	void checkMicroRead();
	void checkMicroWrite();

	static tdcdata::Settings getDefaultSettings();
  private:
	bool			mIsInit;
	int32_t			mVmeHandle;
	int16_t			mBaseAddress;

	tdcdata::Settings mSettings;
};

}//namespace caenTDC

#endif // CAEN_MODULE_H
