#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <cstdint>
#include <list>

#include <boost/asio.hpp>

#include "caenTDC/tdcmodule.hpp"
#include "session.hpp"

#include "handler.hpp"

class Server : public ProcessHandler {
	using SessionPtr = std::shared_ptr<Session>;
	using ModulePtr = std::shared_ptr<caen::Module>;
	using SessionList = std::list<SessionPtr>;
	using IoService = boost::asio::io_service;
	using IpAddress = boost::asio::ip::address;
	using TCP = boost::asio::ip::tcp;
	using Socket = TCP::socket;
	using Acceptor = TCP::acceptor;
	using Endpoint = TCP::endpoint;
public:
	Server(const std::string& ipAdrress, uint16_t port);
protected:
	void workerLoop() override;
	void accept(Socket& socket);
private:
	ModulePtr mDevice;
	IoService mIoService;
	Acceptor mAcceptor;

	SessionList mSessions;
};

#endif // SERVER_HPP
