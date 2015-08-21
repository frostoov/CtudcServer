#ifndef APPLOG_HPP
#define APPLOG_HPP

#include <iostream>
#include <fstream>

#include "timeprint.hpp"

class AppLog {
    using ostream_function = std::ostream& (*)(std::ostream&);
public:
    AppLog(AppLog&& other) = delete;
    AppLog(const AppLog& other) = delete;

    void operator=(const AppLog& other) = delete;
    void operator=(AppLog&& other) = delete;
    static AppLog& instance() {
        static AppLog log;
        return log;
    }

    template<typename T>
    AppLog& operator<<(const T& item) {
        mStream << item;
        std::cout << item;
        return instance();
    }

    AppLog& operator<<(ostream_function func) {
        mStream << func;
        std::cout << func;
        return instance();
    }

    void flush() {
        mStream.flush();
    }

private:
    AppLog() {
        mStream.exceptions(mStream.failbit | mStream.badbit);
        mStream.open("CtudcServer.log", mStream.app | mStream.binary);
    }
    std::ofstream mStream;
};

#endif // APPLOG_HPP
