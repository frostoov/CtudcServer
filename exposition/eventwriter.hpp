#pragma once

#include <trek/data/eventrecord.hpp>
#include <fstream>


class EventWriter {
public:
    EventWriter(const std::string& path,
                const std::string& prefix,
                uintmax_t eventsPerFile);
    void writeEvent(const trek::data::EventRecord& record);
    void writeDrop(const trek::data::EventRecord& record);
protected:
    void reopenStream();
    void openStream();
    std::string formFileName() const;
private:
    std::ofstream mStream;
    std::ofstream mDropStream;
    uintmax_t mFileCount;
    uintmax_t mEventCount;

    const std::string   mPath;
    const std::string   mPrefix;
    const uintmax_t mEventsPerFile;
};
