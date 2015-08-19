#ifndef NETTOOLS
#define NETTOOLS

#include <streambuf>

class membuf : public std::basic_streambuf<char> {
public:
	membuf(char* p, size_t n) {
		setg(p, p, p + n);
		setp(p, p + n);
	}
};

bool verifyNevodPackage(char* data, size_t size);

#endif // NETTOOLS
