#ifndef MAKESTRING_HPP
#define MAKESTRING_HPP

#include <string>
#include <sstream>

class MakeString {
public:
    template<class T>
    MakeString& operator<< (const T& arg) {
        mStream << arg;
        return *this;
    }
    operator std::string() const {
        return mStream.str();
    }
protected:
    std::ostringstream mStream;
};

#endif // MAKESTRING_HPP
