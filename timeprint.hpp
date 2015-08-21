#ifndef TIMEPRINT
#define TIMEPRINT

#include <chrono>
#include <ctime>
#include <iomanip>

template<typename Clock, typename Duration>
std::ostream& operator<<(std::ostream& stream, const std::chrono::time_point<Clock, Duration>& time_point) {
    auto time = Clock::to_time_t(time_point);
    auto tm = std::localtime(&time);
//	return stream << std::put_time(std::localtime(&time), "%F %T %z %Z");
    return stream << tm->tm_year + 1900 << '-' << tm->tm_mon+1 << '-' << tm->tm_mday << ' '
           << tm->tm_hour << ':' << tm->tm_min << ':' << tm->tm_sec;
}

#endif // TIMEPRINT

