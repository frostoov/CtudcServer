#pragma once

#include <vector>
#include <string>
#include <cstddef>

class Tdc {
public:
	struct Hit {
		unsigned channel;
		unsigned time;
	};
	enum class EdgeDetection {
		trailing,
		leading,
	};
	enum class Mode {
		trigger,
		continuous,
	};
	struct Settings {
		unsigned      windowWidth;
		int           windowOffset;
		EdgeDetection edgeDetection;
		unsigned      lsb;
	};
	using EventHits = std::vector<Hit>;
public:
	virtual ~Tdc() {}

	virtual void read(std::vector<EventHits>& buffer) = 0;
	virtual const std::string& name() const = 0;
	virtual Settings settings() = 0;
	virtual bool isOpen() const = 0;
	virtual void clear() = 0;
protected:
	Tdc() = default;
};
