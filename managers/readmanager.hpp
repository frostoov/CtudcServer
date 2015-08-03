#ifndef READMANAGER_HPP
#define READMANAGER_HPP

#include <fstream>
#include <tdcdata/datasetheader.hpp>

#include "modulehandler.hpp"

namespace caen {

class ReadManager : public ProcessManager {
  public:
	ReadManager(ModulePtr module, const std::string& dirName, size_t eventsPerFile,
				const ChannelConfig& channelConfig);

	const char* getTitle() const override {return "ReadManager";}

  protected:
	void workerLoop() override;
	bool start() override;
	void writeTdcRecord(const tdcdata::TDCRecord& event);
	bool needNewStream();
	bool openStream(std::ofstream& stream);
	void closeStream(std::ofstream& stream);

	void resetEventCount()       { mEventCount = 0; }
	void resetFileCount()	 { mFileCount = 0; }

	void increaseEventCount() { ++mEventCount; }
	void increaseFileCount()  { ++mFileCount; }

	void setFileType(tdcdata::DataSetType type) { mFileType = type; }

	uintmax_t getEventCount() const {return mEventCount;}

	std::ofstream mStream;
  private:
	std::string mPath;
	uintmax_t mEventCount;
	uintmax_t mFileCount;
	const uintmax_t mEventsPerFile;



	tdcdata::DataSetType mFileType;
};

} // caen

#endif  // READMANAGER_HPP
