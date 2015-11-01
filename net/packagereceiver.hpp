#pragma once

#include <boost/asio.hpp>
#include <mutex>

class PackageReceiver {
	using UDP       = boost::asio::ip::udp;
	using Endpoint  = UDP::endpoint;
	using UdpSocket = boost::asio::ip::udp::socket;
	using IoService = boost::asio::io_service;
	using IpAddress = boost::asio::ip::address;
public:
	using ByteVector  = std::vector<char>;
	using Callback  = std::function<void (ByteVector&) >;
public:
	PackageReceiver(const std::string& multicastAddress, uint16_t port);
	~PackageReceiver();
	void start();
	void stop();
	void onRecv(Callback&& callback);
protected:
	void doReceive();
	void joinMulticastGroup(const IpAddress& multicastAddress);
	void leaveMulticastGroup(const IpAddress& multicastAddress);
private:
	IoService mIoService;
	UdpSocket mSocket;
	Endpoint  mEndpoint;
	IpAddress mMulticastAddress;

	std::mutex callbackMutex;
	ByteVector mBuffer;
	Callback   mCallback;

	static const size_t mBufferSize = 65527;
};
