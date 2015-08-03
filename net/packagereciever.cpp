#include "packagereciever.hpp"

PackageReciever::PackageReciever(const std::string& multicastAddress, uint16_t port)
	: mSocket(mIoService),
	  mEndpoint(UDP::v4(), port),
	  mMulticastAddress(IpAddress::from_string(multicastAddress)),
	  mHasFunction(false) {
	mSocket.open(UDP::v4());
	mSocket.bind(mEndpoint);
	mIsActive = false;
}

PackageReciever::PackageReciever(const std::string& multicastAddress, uint16_t port, const Callback& function)
	: PackageReciever(multicastAddress, port) {
	setCallback(function);
}

PackageReciever::~PackageReciever() {
	stop();
}

void PackageReciever::start() {
	if(mIsActive == false) {
		mIsActive = true;
		joinMulticastGroup(mMulticastAddress);
		asyncReceive();

		std::thread([this]() {mIoService.run(); } ).detach();
	}
}

void PackageReciever::stop() {
	if(mIsActive == true) {
		leaveMulticastGroup(mMulticastAddress);
		mIoService.stop();
		mIoService.reset();
		mIsActive = false;
	}
}

void PackageReciever::setCallback(const Callback& function) {
	mFunction = function;
	mHasFunction = true;
}

void PackageReciever::clearCallback() {
	mHasFunction = false;
}

bool PackageReciever::isActive() const {
	return mIsActive;
}

void PackageReciever::handleRecive(const boost::system::error_code& error, size_t bytes_transferred) {
	if(!error) {
		asyncReceive();
		if(mHasFunction)
			mFunction(mData, bytes_transferred);
	}
}

void PackageReciever::asyncReceive() {
	mSocket.async_receive_from(boost::asio::buffer(mData, sizeof(mData)), mEndpoint,
							   boost::bind(&PackageReciever::handleRecive, this,
										   boost::asio::placeholders::error,
										   boost::asio::placeholders::bytes_transferred));
}

void PackageReciever::joinMulticastGroup(const IpAddress& multicastAddress) {
	mSocket.set_option(boost::asio::ip::multicast::join_group(multicastAddress));
}

void PackageReciever::leaveMulticastGroup(const IpAddress& multicastAddress) {
	mSocket.set_option(boost::asio::ip::multicast::leave_group(multicastAddress));
}
