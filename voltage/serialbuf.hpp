#pragma once

#include <streambuf>

class serialbuf : public std::streambuf {
public:
	serialbuf();
	serialbuf(const std::string& dev_name);
	~serialbuf();

	void open(const std::string& dev_name);
	void close();
	bool isOpen() const;
protected:
	int sync() override final;

	int_type underflow() override final;
	int_type uflow() override final;
	std::streamsize xsgetn(char* s, std::streamsize size) override final;

	int_type overflow(int_type ch = traits_type::eof()) override final;
	std::streamsize xsputn(const char* s, std::streamsize size) override final;

	void resetBuffers();
	bool setProperties(int file);
private:
	bool updateIBuffer();
	bool updateOBuffer();
private:
	static constexpr size_t mBufferSize = 1024;
	char* mIBuffer;
	char* mOBuffer;

	int mFile;
};
