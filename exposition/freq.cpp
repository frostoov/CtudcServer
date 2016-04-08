#include "freq.hpp"

#include <future>

using std::chrono::microseconds;
using std::chrono::duration_cast;
using std::chrono::system_clock;

using std::make_shared;
using std::logic_error;
using std::shared_ptr;
using std::atomic_bool;
using std::runtime_error;
using std::vector;

static void handleBuffer(const vector<Tdc::Hit>& buffer, ChannelFreq& freq) {
    for(auto& hit : buffer) {
        if(freq.count(hit.channel) != 0)
            ++freq.at(hit.channel);
        else
            freq.emplace(hit.channel, 1);
    }
}

std::function<ChannelFreq()> launchFreq(shared_ptr<Tdc> tdc, microseconds delay) {
    using Future = std::future<ChannelFreq>;
    if(!tdc->isOpen())
        throw logic_error("measureFreq tdc is not open");
    auto active = make_shared<atomic_bool>(true);
    
    auto future = make_shared<Future>(std::async(std::launch::async, [=] {
        ChannelFreq freq;
        auto duration = microseconds::zero();
        vector<Tdc::Hit> buffer;
        while(active->load()) {
            tdc->clear();
            auto s = system_clock::now();
            std::this_thread::sleep_for(delay);
            duration += duration_cast<microseconds>(system_clock::now() - s);
            tdc->readHits(buffer);
            handleBuffer(buffer, freq);
        }
        for(auto& countPair : freq)
            countPair.second *= (1000000.0/duration.count());
        return freq;
    }));

    return [=] {
        active->store(false);
        return future->get();
    };
}
