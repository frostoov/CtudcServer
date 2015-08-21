#ifndef SESSION_HPP
#define SESSION_HPP

#include <functional>
#include <string>
#include <array>
#include <unordered_map>

#include <boost/asio.hpp>

#include "controller/controller.hpp"

class Session : public std::enable_shared_from_this<Session> {
    using CloseCallback = std::function<void (std::shared_ptr<Session>) >;
    using ControllerPtr = std::shared_ptr<Controller>;
    using Controllers = std::unordered_map<std::string, ControllerPtr>;
    using TCP = boost::asio::ip::tcp;
    using Socket = TCP::socket;
public:
    Session(const Controllers& controllers, Socket&& socket, CloseCallback closeCallback);
    void start();
protected:
    void doRecieve();
    void doSend(const std::string& response);
    Controller& getController(const std::string request);
private:
    Controllers mControllers;
    Socket mSocket;
    CloseCallback mCloseCallback;
    std::array<char, 65527> mBuffer;
};

#endif // SESSION_HPP
