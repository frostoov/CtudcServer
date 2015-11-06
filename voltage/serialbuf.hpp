#pragma once

#include <streambuf>

class serialbuf : public std::streambuf {
public:
	serialbuf();
	serialbuf(const std::string& dev_name, unsigned baudRate);
	~serialbuf();

	void open(const std::string& dev_name, unsigned baudRate);
	void close();
	bool isOpen() const;

	void setTimeout(int timeout);
protected:
	int sync() override final;

	int_type underflow() override final;
	int_type uflow() override final;
	std::streamsize xsgetn(char* s, std::streamsize size) override final;

	int_type overflow(int_type ch = traits_type::eof()) override final;
	std::streamsize xsputn(const char* s, std::streamsize size) override final;

	void resetBuffers();
	bool setProperties(int file, unsigned baudRate);
private:
	bool updateIBuffer();
	bool updateOBuffer();
private:
	static constexpr size_t mBufferSize = 1024;
	int mTimeout;
	char* mIBuffer;
	char* mOBuffer;

	int mFile;
};
