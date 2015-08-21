#include <json.hpp>

#include "controller/ctudccontroller.hpp"

#include "session.hpp"
#include "applog.hpp"
#include "makestring.hpp"

using std::string;
using std::array;
using std::endl;
using std::chrono::system_clock;

using nlohmann::json;
using boost::system::error_code;


Session::Session(const Controllers& controllers, Socket&& socket, CloseCallback closeCallback)
    : mControllers(controllers),
      mSocket(std::move(socket)),
      mCloseCallback(closeCallback) { }

void Session::start() {
    doRecieve();
}

void Session::doRecieve() {
    mSocket.async_receive(boost::asio::buffer(mBuffer), [this](error_code errCode, size_t length) {
        if(!errCode) {
            AppLog::instance() << system_clock::now() << " Received request:\n";
            string response;
            try {
                string requset(mBuffer.data(), length);
                AppLog::instance() << requset << endl;
                auto& controller = getController(requset);
                response = controller.handleRequest(requset);
            } catch(const std::exception& e) {
                response = string(MakeString() << "Invalid query " << e.what());
            }
            doSend(response);
            doRecieve();
        } else {
            AppLog::instance() << "Disconnected: " << mSocket.remote_endpoint().address().to_string() << endl;
            mSocket.close();
            mCloseCallback(shared_from_this());
        }
    });
}

void Session::doSend(const std::string& response) {
    auto self(shared_from_this());
    mSocket.async_send(boost::asio::buffer(response), [this, self](error_code errCode, size_t) {
        if(errCode)
            mCloseCallback(self);
    });
}

Controller& Session::getController(const std::string request) {
    auto object = ctudc::CtudcController::getObject(request);
    return *mControllers.at(object);
}
