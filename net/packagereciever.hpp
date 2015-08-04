#ifndef PACKAGERECIEVER_HPP
#define PACKAGERECIEVER_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <thread>

#include <tdcdata/structs.hpp>


class PackageReciever {
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
	void start();
	void stop();
	void setCallback(const Callback& function);
	void clearCallback();
	bool isActive() const;
  protected:
	void doReceive();
	void joinMulticastGroup(const IpAddress& multicastAddress);
	void leaveMulticastGroup(const IpAddress& multicastAddress);
  private:
	IoService mIoService;
	UdpSocket mSocket;
	Endpoint  mEndpoint;

	static const size_t mDataMaxSize = 65527;
	char mData[mDataMaxSize];

	IpAddress mMulticastAddress;

	Callback mFunction;
	volatile bool mHasFunction;

	volatile bool mIsActive;
};

#endif // PACKAGERECIEVER_HPP
