#include "serialbuf.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <sys/stat.h>

#include <cstring>
#include <thread>
#include <iostream>
#include <unordered_map>


namespace chrono = std::chrono;
using std::size_t;
using std::runtime_error;
using std::streamsize;
using std::ptrdiff_t;
using std::memcpy;

const std::unordered_map<unsigned, int> baudRates{
	{ 0,      B0 },
	{ 50,     B50},
	{ 75,     B75},
	{ 110,    B110},
	{ 134,    B134},
	{ 150,    B150},
	{ 200,    B200},
	{ 300,    B300},
	{ 600,    B600},
	{ 1200,   B1200},
	{ 1800,   B1800},
	{ 2400,   B2400},
	{ 4800,   B4800},
	{ 9600,   B9600},
	{ 19200,  B19200},
	{ 38400,  B38400},
	{ 57600,  B57600},
	{ 115200, B115200},
	{ 230400, B230400},
};

serialbuf::serialbuf()
	: mIBuffer(new char[mBufferSize]),
	  mOBuffer(new char[mBufferSize]),
	  mTimeout(-1),
	  mFile(-1) { }

serialbuf::serialbuf(const std::string& dev_name, unsigned baudRate)
	: mIBuffer(new char[mBufferSize]),
	  mOBuffer(new char[mBufferSize]),
	  mTimeout(-1),
	  mFile(-1) {
	open(dev_name, baudRate);
}

serialbuf::~serialbuf() {
	delete[] mIBuffer;
	delete[] mOBuffer;
	close();
}

void serialbuf::open(const std::string& dev_name, unsigned baudRate) {
	if (isOpen())
		throw runtime_error("serialbuf::open device already opened");
	mFile = ::open(dev_name.data(), O_RDWR | O_NOCTTY);
	if (mFile == -1)
		throw runtime_error("serialbuf::open failed open device");
	if (!setProperties(mFile, baudRate)) {
		close();
		throw runtime_error("serialbuf::open failed setup device");
	}
	std::this_thread::sleep_for(chrono::seconds(2));
	resetBuffers();
}

void serialbuf::close() {
	if (isOpen() && ::close(mFile) == -1)
		throw runtime_error("serialbuf::open failed close device");
	mFile = -1;
}

bool serialbuf::isOpen() const {
	return mFile != -1;
}


serialbuf::int_type serialbuf::underflow() {
	if (!updateIBuffer())
		return traits_type::eof();
	return serialbuf::int_type(*gptr());
}

serialbuf::int_type serialbuf::uflow() {
	auto c = underflow();
	if (c != traits_type::eof())
		gbump(1);
	return c;
}

streamsize serialbuf::xsgetn(char* s, streamsize size) {
	if (gptr() == egptr() && !updateIBuffer())
		return traits_type::eof();
	auto transfer = std::min(streamsize(egptr() - gptr()), size);
	if (transfer <= 0)
		return 0;
	memcpy(s, gptr(), size_t(transfer));
	gbump(int(transfer));
	return transfer;
}

int serialbuf::sync() {
	if (!updateOBuffer() )
		return -1;
	return 0;
}

serialbuf::int_type serialbuf::overflow(int_type ch) {
	if (!updateOBuffer())
		return traits_type::eof();
	*pptr() = char(ch);
	return 0;
}

streamsize serialbuf::xsputn(const char* s, streamsize size) {
	streamsize bytesWritten = 0;
	while (bytesWritten != size) {
		if (pptr() == epptr() && !updateOBuffer())
			return traits_type::eof();
		auto transfer = std::min(size - bytesWritten, streamsize(epptr() - pptr()));
		memcpy(pptr(), s + bytesWritten, size_t(transfer));
		pbump(int(transfer));
		bytesWritten += transfer;
	}
	return bytesWritten;
}

bool serialbuf::updateIBuffer() {
	pollfd fd = {mFile, POLLIN, 0};
	if( poll(&fd, 1, mTimeout ) != 1 )
		return false;

	auto transfer = ::read(mFile, mIBuffer, mBufferSize);
	if (transfer == -1)
		return false;
	setg(mIBuffer, mIBuffer, mIBuffer + transfer);
	return true;
}

bool serialbuf::updateOBuffer() {
	pollfd fd = {mFile, POLLOUT, 0};
	if( poll(&fd, 1, mTimeout) != 1 )
		return false;

	auto transfer = ::write(mFile, pbase(), size_t(pptr() - pbase()));
	if (transfer != pptr() - pbase())
		return false;
	setp(mOBuffer, mOBuffer + mBufferSize);
	return true;
}

void serialbuf::resetBuffers() {
	setg(mIBuffer, mIBuffer, mIBuffer);
	setp(mOBuffer, mOBuffer + mBufferSize);
}

bool serialbuf::setProperties(int file, unsigned baudRate) {
	struct termios properties;
	if (tcgetattr(file, &properties) == -1)
		return false;
	properties.c_cflag = CS8 | baudRates.at(baudRate) | CREAD | CLOCAL;
	properties.c_iflag = IGNBRK;
	properties.c_oflag = 0;
	properties.c_lflag = 0;

	properties.c_cc[VTIME] = 0;   /* inter-character timer unused */
	properties.c_cc[VMIN] = 1;

	if (tcsetattr(file, TCSANOW, &properties) == -1)
		return false;
	return tcflush(file, TCIOFLUSH) != -1;
}

void serialbuf::setTimeout(int timeout) {
	mTimeout = timeout;
}
