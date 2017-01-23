#include "ihepexposition.hpp"
#include "eventwriter.hpp"

#include <memory>
#include <vector>
#include <chrono>
#include <iostream>

using trek::data::EventHits;

using std::shared_ptr;
using std::vector;
using std::chrono::microseconds;
using std::cout;
using std::endl;

using std::exception;


IHEPExposition::IHEPExposition(shared_ptr<Tdc> tdc,
                               const Settings& settings,
                               const ChannelConfig& config)
    : mTrgCount(0),
      mActive(true) {
    if(!tdc->isOpen())
        throw std::logic_error("launchExpo tdc is not open");
    tdc->reset();
    tdc->clear();
    mReadThread = std::thread([this, tdc, settings, config]{
        printStartMeta(settings.writeDir, settings.nRun, *tdc);
        this->readLoop(tdc, settings, config);
        printEndMeta(settings.writeDir, settings.nRun);
    });
}

IHEPExposition::~IHEPExposition() {
    stop();
}

void IHEPExposition::stop() {
    mActive = false;
    mCv.notify_one();
    mReadThread.join();
}

void IHEPExposition::readLoop(shared_ptr<Tdc> tdc, const Settings& settings, const ChannelConfig chanConf) {
    vector<Tdc::EventHits> buffer;
    EventWriter eventWriter(runPath(settings.writeDir, settings.nRun),
                            formatPrefix(settings.nRun),
                            settings.eventsPerFile);
    unsigned num = 0;
    while(mActive) {
        std::mutex m;
        std::unique_lock<std::mutex> lk(m);
        mCv.wait_for(lk, microseconds(settings.readFreq));
        try {
            tdc->readEvents(buffer);
            std::cout << "transfered: " << buffer.size() << std::endl;
            for(auto& e : handleEvents(buffer, chanConf)) {
                eventWriter.writeEvent({settings.nRun, num++, e});
            }
        } catch(exception& e) {
            std::cerr << "ATTENTION!!! ihep read loop failure " << e.what() << std::endl;
            try {
                tdc->reset();
            } catch(exception& e) {
                std::cerr << "Failed reset tdc " << tdc->name() << std::endl;
            }
        }
    }
}

vector<EventHits> IHEPExposition::handleEvents(const EventBuffer& buffer, const ChannelConfig& conf) {
    auto events = convertEvents(buffer, conf);
    mTrgCount += events.size();
    for(auto& event : events) {
        for(auto& hit : event) {
            if(mChambersCount.count(hit.chamber()) == 0)
               mChambersCount.emplace(hit.chamber(), ChamberHitCount{{0, 0, 0, 0}});
            ++mChambersCount.at(hit.chamber()).at(hit.wire());
        }
    }
    return events;
}
