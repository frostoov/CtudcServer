#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <cstdint>
#include <list>
#include <thread>

#include <boost/asio.hpp>

#include "controller/controller.hpp"
#include "session.hpp"

class Server {
    using ThreadPtr  = std::unique_ptr<std::thread>;
    using SessionPtr = std::shared_ptr<Session>;
    using SessionList = std::list<SessionPtr>;
    using ControllerPtr = std::shared_ptr<Controller>;
    using Controllers   = std::unordered_map<std::string, ControllerPtr>;
    using IoService = boost::asio::io_service;
    using IpAddress = boost::asio::ip::address;
    using TCP = boost::asio::ip::tcp;
    using UDP = boost::asio::ip::udp;
public:
    Server(const Controllers& controllers, const std::string& ipAdrress, uint16_t port);
    ~Server();
    bool start();
    void stop();
protected:
    void doAccept();
    void receiveNevodSignal();
private:
    Controllers mControllers;
    IoService mIoService;
    TCP::acceptor mAcceptor;
    TCP::socket mSocket;

    SessionList mSessions;
    ThreadPtr mThread;
};

#endif // SERVER_HPP
