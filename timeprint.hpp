#ifndef TIMEPRINT
#define TIMEPRINT

#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

template<typename Clock, typename Duration>
std::ostream &operator<<(std::ostream &stream, const std::chrono::time_point<Clock, Duration> &time_point) {
    auto time = Clock::to_time_t(time_point);
    return stream << std::put_time(std::localtime(&time), "%F %T %z %Z");
}

#endif // TIMEPRINT

