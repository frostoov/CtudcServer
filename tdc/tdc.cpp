#include "tdc.hpp"

#include <iostream>

std::ostream& operator<<(std::ostream& stream, Tdc::EdgeDetection ed) {
    switch(ed) {
    case Tdc::EdgeDetection::leading:
        return stream << "leading";
    case Tdc::EdgeDetection::trailing:
        return stream << "trailing";
    case Tdc::EdgeDetection::leadingTrailing:
        return stream << "leading_trailing";
    }
}

std::ostream& operator<<(std::ostream& stream, Tdc::Mode mode) {
    switch(mode) {
    case Tdc::Mode::trigger:
        return stream << "trigger";
    case Tdc::Mode::continuous:
        return stream << "continuous";
    }
}


std::ostream& operator<<(std::ostream& stream, const Tdc::Settings& settings) {
    stream << "LSB:            " << settings.lsb << '\n';
    stream << "Edge Detection: " << settings.edgeDetection << '\n';
    stream << "Window width:   " << settings.windowWidth << '\n';
    stream << "Window offset:  " << settings.windowOffset << '\n';
    return stream;
}
