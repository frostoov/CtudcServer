#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <cstdint>
#include <list>

#include <boost/asio.hpp>

#include "session.hpp"

class Server {
	using SessionPtr = std::shared_ptr<Session>;
	using DeviceManagerPtr = std::shared_ptr<DeviceManager>;
	using SessionList = std::list<SessionPtr>;
	using IoService = boost::asio::io_service;
	using IpAddress = boost::asio::ip::address;
	using TCP = boost::asio::ip::tcp;
	using Socket = TCP::socket;
	using Acceptor = TCP::acceptor;
	using Endpoint = TCP::endpoint;
public:
	Server(DeviceManagerPtr deviceManager, const std::string& ipAdrress, uint16_t port);
	void start();
	void stop();
protected:
	void doAccept();
private:
	DeviceManagerPtr mDeviceManager;
	IoService mIoService;
	Acceptor mAcceptor;
	Socket mSocket;

	SessionList mSessions;
};

#endif // SERVER_HPP
