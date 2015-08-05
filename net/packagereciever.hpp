#ifndef PACKAGERECIEVER_HPP
#define PACKAGERECIEVER_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <thread>

#include <tdcdata/structs.hpp>

#include "cppchannel/channel"

class PackageReciever {
	using ByteVector  = std::vector<char>;
	using ThreadPtr   = std::unique_ptr<std::thread>;
	using StopChannel = cpp::channel<bool>;
	using DataChannel = cpp::channel<std::vector<char>>;
	using UDP       = boost::asio::ip::udp;
	using Endpoint  = UDP::endpoint;
	using UdpSocket = boost::asio::ip::udp::socket;
	using IoService = boost::asio::io_service;
	using IpAddress = boost::asio::ip::address;
	using Callback  = std::function<void(char* data, size_t size)>;
  public:
	PackageReciever(const std::string& multicastAddress, uint16_t port);
	PackageReciever(const std::string& multicastAddress, uint16_t port, const Callback& function);
	~PackageReciever();
	bool start();
	void stop();
	void clearCallback();
	bool isActive() const;
	DataChannel getDataChannel() const;
  protected:
	void doReceive();
	void joinMulticastGroup(const IpAddress& multicastAddress);
	void leaveMulticastGroup(const IpAddress& multicastAddress);
  private:
	IoService mIoService;
	UdpSocket mSocket;
	Endpoint  mEndpoint;
	IpAddress mMulticastAddress;

	ByteVector mBuffer;
	ThreadPtr mThread;

	static const size_t mBufferSize = 65527;

	DataChannel mDataChannel;

	volatile bool mIsActive;
};

#endif // PACKAGERECIEVER_HPP
