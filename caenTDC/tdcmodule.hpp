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

	void initialize();
	void close();
	void setSettings(const tdcdata::Settings& mSettings);
	void setLSB(tdcdata::Lsb lsb);
	void setWindowWidth(uint16_t windowWidth);
	void setWindowOffset(int16_t winOffset);
	void setAlmostFull(uint16_t value);
	void setDetection(tdcdata::EdgeDetection edgeDetection);
	void setControl(uint16_t control);
	void setDeadTime(uint16_t deadTime);
	void setEventBLT(uint16_t eventBLT);
	void setTriggerMode(bool flag);
	void setTriggerSubtraction(bool flag);
	void setTDCMeta(bool flag);

	void updateMode();
	void updateTdcMeta();
	void updateDetection();
	void updateDeadTime();
	void updateLSB();
	void updateStatus();
	void updateControl();
	void updateAlmostFull();
	void updateEventBLT();
	void updateTriggerConfig();


	uint16_t getFirmwareRev();
	uint16_t getMicroRev();
	void     softwareClear();

	size_t readBlock(WordVector& buff, const Microseconds& delay = Microseconds::zero());
	size_t getBlockSize() const;

	void updateSettings();
	const tdcdata::Settings& getSettings() const { return mSettings; }
  protected:
	void init();
	uint32_t readReg32(Reg addr) const;
	uint16_t readReg16(Reg addr) const;
	void writeReg32(uint32_t data, Reg addr);
	void writeReg16(uint16_t data, Reg addr);

	void readMicro	(uint16_t* data, OpCode code, short num = 1);
	void writeMicro	(uint16_t* data, OpCode code, short num = 1);

	void doAction(const std::string& message, std::function<void()> func);

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
