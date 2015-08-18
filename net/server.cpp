#include <thread>
#include "server.hpp"
#include "nettools.hpp"

using std::string;
using std::cerr;
using std::cout;
using std::endl;
using std::exception;
using std::make_shared;
using std::thread;
using std::istream;

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
}

void Server::stop() {
    mSessions.clear();
    mAcceptor.cancel();
    mIoService.stop();
}

void Server::doAccept() {
    mAcceptor.async_accept(mSocket, [this](boost::system::error_code errCode) {
        if(!errCode) {
            cout << "Accepted connection: " << mSocket.remote_endpoint().address().to_string() << endl;
            SessionPtr newSession = make_shared<Session> (mDeviceManager, std::move(mSocket), [this](SessionPtr session) {
                mSessions.remove(session);
            });
            mSessions.push_back(newSession);
            newSession->start();
        } else
            cerr << "error: " << errCode.message() << endl;
        doAccept();
    });
}
