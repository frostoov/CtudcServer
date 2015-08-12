#ifndef SESSION_HPP
#define SESSION_HPP

#include <functional>
#include <string>
#include <array>

#include <boost/asio.hpp>

#include "managers/facilitymanager.hpp"

class Session : public std::enable_shared_from_this<Session> {
	using DestroyCallback = std::function<void (std::shared_ptr<Session>) >;
	using DeviceMangerPtr = std::shared_ptr<FacilityManager>;
	using ModulePtr = std::shared_ptr<caen::Module>;
	using TCP = boost::asio::ip::tcp;
	using Socket = TCP::socket;
public:
	Session (DeviceMangerPtr deviceManager, Socket&& socket, DestroyCallback callback);
	void start();
protected:
	void doRecieve();
	void doSend (const std::string& response);
private:
	DeviceMangerPtr mDeviceManager;
	Socket mSocket;
	DestroyCallback mCallback;
	std::array<char, 65527> mBuffer;
};

#endif // SESSION_HPP
