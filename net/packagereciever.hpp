#ifndef PACKAGERECIEVER_HPP
#define PACKAGERECIEVER_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <thread>

#include <tdcdata/structs.hpp>

#include "channel.hpp"

class PackageReceiver {
	using ByteVector  = std::vector<char>;
	using ThreadPtr   = std::unique_ptr<std::thread>;
	using StopChannel = Channel<bool>;
	using DataChannel = Channel<std::vector<char>>;
	using UDP       = boost::asio::ip::udp;
	using Endpoint  = UDP::endpoint;
	using UdpSocket = boost::asio::ip::udp::socket;
	using IoService = boost::asio::io_service;
	using IpAddress = boost::asio::ip::address;
	using Callback  = std::function<void(char* data, size_t size)>;
  public:
	PackageReceiver(const std::string& multicastAddress, uint16_t port);
	PackageReceiver(const std::string& multicastAddress, uint16_t port, const Callback& function);
	~PackageReceiver();
	bool start();
	void stop();
	void clearCallback();
	bool isActive() const;
	DataChannel& getDataChannel();
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

	std::atomic_bool mIsActive;
};

#endif // PACKAGERECIEVER_HPP
