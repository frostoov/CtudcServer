#ifndef READMANAGER_HPP
#define READMANAGER_HPP

#include <fstream>
#include <trekdata/datasetheader.hpp>

#include "processmanager.hpp"

namespace caen {

class ReadManager : public ProcessManager {
public:
    ReadManager(ModulePtr module,
                const ChannelConfig& channelConfig,
                const std::string& dirName,
                uint64_t numberOfRun,
                uintmax_t eventsPerFile);

protected:
    bool init() override;
    void shutDown() override;
    void workerFunc() override;
    void writeTdcRecord(const trekdata::TdcRecord& event);
    bool needNewStream();
    bool openStream(std::ofstream& stream);
    void closeStream(std::ofstream& stream);

    void resetRecordCount();
    void resetFileCount();

    void increaseRecordCount();
    void increaseFileCount();

    void setFileType(trekdata::DataSetType type);

    uintmax_t getNumberOfRecord() const;
    uint64_t  getNumberOfRun() const;
    std::string formFileName() const;

    std::ofstream mStream;
    WordVector mBuffer;
private:
    std::string mPath;
    uint64_t mNumberOfRun;
    uintmax_t mNumberOfRecord;
    uintmax_t mNumberOfFiles;
    const uintmax_t mEventsPerFile;

    trekdata::DataSetType mFileType;
};

} // caen

#endif  // READMANAGER_HPP
