#include <thread>

#include "applog.hpp"
#include "server.hpp"
#include "nettools.hpp"

using std::string;
using std::endl;
using std::exception;
using std::make_shared;
using std::thread;
using std::istream;
using std::chrono::system_clock;

using boost::asio::ip::address;

using caen::Module;

Server::Server(DeviceManagerPtr deviceManager, const string& ipAdrress, uint16_t port)
    : mDeviceManager(deviceManager),
      mAcceptor(mIoService, TCP::endpoint(IpAddress::from_string(ipAdrress), port)),
      mSocket(mIoService) { };

Server::~Server() {
    stop();
}

void Server::start() {
    mIoService.reset();
    doAccept();
    thread([this]() {
        mIoService.run();
    }).detach();
    AppLog::instance() << system_clock::now() << " Server start" << endl;
}

void Server::stop() {
    mSessions.clear();
    mAcceptor.cancel();
    mIoService.stop();
    AppLog::instance() << system_clock::now() << " Server stop" << endl;
}

void Server::doAccept() {
    mAcceptor.async_accept(mSocket, [this](boost::system::error_code errCode) {
        AppLog::instance() << system_clock::now() << ' ';
        if(!errCode) {
            AppLog::instance() << "Accepted connection: " << mSocket.remote_endpoint().address().to_string() << std::endl;
            SessionPtr newSession = make_shared<Session> (mDeviceManager, std::move(mSocket), [this](SessionPtr session) {
                mSessions.remove(session);
            });
            mSessions.push_back(newSession);
            newSession->start();
        } else
            AppLog::instance() << "Failed accept connection: " << errCode.message() << endl;
        doAccept();
    });
}
