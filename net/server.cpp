#include "server.hpp"
#include "applog.hpp"
#include "nettools.hpp"

using std::string;
using std::endl;
using std::exception;
using std::make_shared;
using std::thread;
using std::istream;
using std::make_unique;
using std::chrono::system_clock;

using boost::asio::ip::address;

Server::Server(const Controllers& controllers, const string& ipAdrress, uint16_t port)
    : mControllers(controllers),
      mAcceptor(mIoService, TCP::endpoint(IpAddress::from_string(ipAdrress), port)),
      mSocket(mIoService) { }

Server::~Server() {
    stop();
}

bool Server::start() {
    if(!mThread) {
        mIoService.reset();
        doAccept();
        mThread = make_unique<thread>([this]() { mIoService.run(); });
        AppLog::instance() << system_clock::now() << " Server start" << endl;
        return true;
    } else
        return false;
}

void Server::stop() {
    if(mThread) {
        mIoService.stop();
        mSessions.clear();
        mThread->join();
        mThread.reset();
        AppLog::instance() << system_clock::now() << " Server stop" << endl;
    }
}

void Server::doAccept() {
    mAcceptor.async_accept(mSocket, [this](boost::system::error_code errCode) {
        AppLog::instance() << system_clock::now() << ' ';
        if(!errCode) {
            AppLog::instance() << "Accepted connection: " << mSocket.remote_endpoint().address().to_string() << std::endl;
            auto newSession = make_shared<Session> (mControllers, std::move(mSocket), [this](SessionPtr session) {
                mSessions.remove(session);
            });
            mSessions.push_back(newSession);
            newSession->start();
        } else
            AppLog::instance() << "Failed accept connection: " << errCode.message() << endl;
        mSocket = TCP::socket(mIoService);
        doAccept();
    });
}
