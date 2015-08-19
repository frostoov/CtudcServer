#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <cstdint>
#include <list>

#include <boost/asio.hpp>

#include "session.hpp"

class Server {
	using SessionPtr = std::shared_ptr<Session>;
	using DeviceManagerPtr = std::shared_ptr<FacilityManager>;
	using SessionList = std::list<SessionPtr>;
	using IoService = boost::asio::io_service;
	using IpAddress = boost::asio::ip::address;
	using TCP = boost::asio::ip::tcp;
	using UDP = boost::asio::ip::udp;
public:
	Server(DeviceManagerPtr deviceManager, const std::string& ipAdrress, uint16_t port);
	~Server();
	void start();
	void stop();
protected:
	void doAccept();
	void receiveNevodSignal();
private:
	DeviceManagerPtr mDeviceManager;
	IoService mIoService;
	TCP::acceptor mAcceptor;
	TCP::socket mSocket;

	SessionList mSessions;
};

#endif // SERVER_HPP
