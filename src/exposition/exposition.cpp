#include "exposition.hpp"
#include "eventwriter.hpp"

#include <trek/common/stringbuilder.hpp>
#include <trek/common/timeprint.hpp>
#include <trek/data/nevod.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <future>

using std::string;
using std::setw;
using std::setfill;
using std::chrono::system_clock;

using mkstr = trek::StringBuilder;

namespace fs = boost::filesystem;

string formatPrefix(unsigned run) {
    return mkstr() << "ctudc_" << setw(5) << setfill('0') << run << '_';
}

string runPath(const string& dir, unsigned run) {
    auto runDir = dir / fs::path(mkstr() << "run_" << setw(5) << setfill('0') << run);
    if(!fs::exists(runDir))
        fs::create_directories(runDir);
    else if(!fs::is_directory(runDir))
        throw std::runtime_error("runPath invalid dir");
    return runDir.string();
}

string metaPath(const string& dir, unsigned run) {
    return (fs::path(runPath(dir, run)) / "meta").string();
}

void printStartMeta(const string& dir, unsigned run, Tdc& module) {
    auto filename = metaPath(dir, run);
    std::ofstream stream;
    stream.exceptions(stream.failbit | stream.badbit);
    stream.open(filename, stream.binary | stream.trunc);
    stream << "Run: " << run << '\n';
    stream << "Time: " << system_clock::now() << '\n';
    stream << "TDC: " << module.name() << '\n';
    stream << module.settings();
}

void printEndMeta(const string& dir, unsigned run) {
    auto filename = metaPath(dir, run);
    std::ofstream stream;
    stream.exceptions(stream.failbit | stream.badbit);
    stream.open(filename, stream.binary | stream.app);
    stream << "Stopped: " << system_clock::now();
}

TrekFreq convertFreq(const ChannelFreq& freq, const ChannelConfig& conf) {
    TrekFreq newFreq;
    for(auto& chanFreq : freq) {
        auto& c = conf.at(chanFreq.first);
        if(newFreq.count(c.chamber) == 0)
            newFreq.emplace(c.chamber, ChamberFreq{{0, 0, 0, 0}});
        newFreq.at(c.chamber).at(c.wire) = chanFreq.second;
    }
    return newFreq;
}
