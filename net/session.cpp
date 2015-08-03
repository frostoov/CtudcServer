#include <json.hpp>
#include <iostream>
#include "session.hpp"

using std::string;
using std::array;
using std::cout;
using std::endl;
using std::cerr;

using nlohmann::json;
using boost::system::error_code;


Session::Session(DeviceMangerPtr deviceManager, Socket&& socket, DestroyCallback callback)
	: mDeviceManager(deviceManager),
	  mSocket(std::move(socket)),
	  mCallback(callback) { }

void Session::start()
{
	doRecieve();
}

void Session::doRecieve() {
	auto self(shared_from_this());
	mSocket.async_receive(boost::asio::buffer(mBuffer), [this, self](error_code errCode, size_t length) {
		string response;
		if(!errCode) {
			try {
				string query(mBuffer.data(), length);
				cout << "Received query:\n" << query << endl;
				response = mDeviceManager->handleQuery(query);
			} catch(const std::exception& e) {
				response = "Invalid query";
			}
			doSend(response);
			doRecieve();
		} else {
			cout << "Disconnected: " << mSocket.remote_endpoint().address().to_string() << endl;
			mCallback(self);
		}
	});
}

void Session::doSend(const std::string& response) {
	auto self(shared_from_this());
	mSocket.async_send(boost::asio::buffer(response),[this, self](error_code errCode, size_t length) {
		if(errCode) {
			mCallback(self);
		}
	});
}
