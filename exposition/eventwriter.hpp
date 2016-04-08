#pragma once

#include <trek/data/eventrecord.hpp>
#include <fstream>


class EventWriter {
public:
    EventWriter(const std::string& path,
                const std::string& prefix,
                unsigned eventsPerFile);
    void writeEvent(const trek::data::EventRecord& record);
    void writeDrop(const trek::data::EventRecord& record);
protected:
    void reopenStream();
    void openStream();
    std::string formFileName() const;
private:
    std::ofstream mStream;
    std::ofstream mDropStream;
    unsigned      mFileCount;
    unsigned      mEventCount;

    const std::string   mPath;
    const std::string   mPrefix;
    const unsigned      mEventsPerFile;
};
