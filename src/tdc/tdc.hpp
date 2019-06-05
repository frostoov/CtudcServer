#pragma once

#include <vector>
#include <string>
#include <cstddef>

class Tdc {
public:
    enum class EdgeDetection {
        leading = 0,
        trailing = 1,
        leadingTrailing = 2,
    };
    enum class Mode {
        trigger    = 0,
        continuous = 1,
    };
    struct Hit {
        Hit(EdgeDetection ed, unsigned c, unsigned t) : type(ed), channel(c), time(t) {}
        EdgeDetection type;
        unsigned channel;
        unsigned time;
    };
    struct Settings {
        unsigned      windowWidth;
        int           windowOffset;
        EdgeDetection edgeDetection;
        unsigned      lsb;
    };
    using EventHits = std::vector<Hit>;
public:
    virtual ~Tdc() { }

    virtual void readEvents(std::vector<EventHits>& buffer) = 0;
    virtual void readHits(std::vector<Hit>& buffer) = 0;
    virtual const std::string& name() const = 0;
    virtual Settings settings() = 0;
    virtual bool isOpen() const = 0;
    virtual void clear() = 0;
    virtual void reset() = 0;
    virtual Mode mode() = 0;
    virtual void setMode(Mode mode) = 0;
protected:
    Tdc() = default;
};


std::ostream& operator<<(std::ostream& stream, Tdc::EdgeDetection ed);
std::ostream& operator<<(std::ostream& stream, Tdc::Mode mode);
std::ostream& operator<<(std::ostream& stream, const Tdc::Settings& settings);
