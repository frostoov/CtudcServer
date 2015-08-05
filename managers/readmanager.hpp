#ifndef READMANAGER_HPP
#define READMANAGER_HPP

#include <fstream>
#include <tdcdata/datasetheader.hpp>

#include "processmanager.hpp"

namespace caen {

class ReadManager : public ProcessManager {
  public:
	ReadManager(ModulePtr module, const std::string& dirName, size_t eventsPerFile,
	            const ChannelConfig& channelConfig);

  protected:
	bool init() override;
	void shutDown() override;
	void flush() override;
	void workerFunc() override;
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
	WordVector mBuffer;
  private:
	std::string mPath;
	uintmax_t mEventCount;
	uintmax_t mFileCount;
	const uintmax_t mEventsPerFile;

	tdcdata::DataSetType mFileType;
};

} // caen

#endif  // READMANAGER_HPP
