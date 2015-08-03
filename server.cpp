#include <thread>
#include <iostream>
#include "server.hpp"

using std::string;
using std::thread;
using std::cerr;
using std::cout;
using std::endl;
using std::exception;
using std::make_shared;

using boost::asio::ip::address;

using caen::Module;

Server::Server(const string& ipAdrress, uint16_t port)
	: mDevice(make_shared<Module>(0xEE00)),
	  mAcceptor(mIoService, Endpoint(IpAddress::from_string(ipAdrress), port))
{}

void Server::workerLoop() {
	setLoop(true);
	while(isActive()) {
		try {
			Socket socket(mIoService);
			accept(socket);
			cout << "Accepted connection: " << socket.remote_endpoint().address().to_string() << endl;
			SessionPtr newSession = make_shared<Session>(mDevice, std::move(socket), [this](SessionPtr session) {
				mSessions.remove(session);
			});
			mSessions.push_back(newSession);
			newSession->start();
		} catch(const exception& e) {
			cerr << "Error during acception: " << e.what() << endl;
		}
	}
	setLoop(false);
	mSessions.clear();
}

void Server::accept(Socket& socket) {
	mAcceptor.accept(socket);
}

