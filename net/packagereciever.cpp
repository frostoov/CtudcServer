#include "packagereciever.hpp"

using boost::system::error_code;

PackageReciever::PackageReciever(const std::string& multicastAddress, uint16_t port)
	: mSocket(mIoService),
	  mEndpoint(UDP::v4(), port),
	  mMulticastAddress(IpAddress::from_string(multicastAddress)),
	  mHasFunction(false) {
	mSocket.open(UDP::v4());
	mSocket.bind(mEndpoint);

	mSocket.set_option(UDP::socket::receive_buffer_size(sizeof(mData)));
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
		doReceive();

		std::thread([this]() {mIoService.run(); } ).detach();
	}
}

void PackageReciever::stop() {
	if(mIsActive == true) {
		mIoService.stop();
		leaveMulticastGroup(mMulticastAddress);
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

void PackageReciever::doReceive() {
	mSocket.async_receive_from(boost::asio::buffer(mData, sizeof(mData)), mEndpoint,
							   [&, this](const error_code& error, size_t size) {
		if(!error) {
			if(mHasFunction)
				mFunction(mData, size);
		}
		doReceive();
	});
}

void PackageReciever::joinMulticastGroup(const IpAddress& multicastAddress) {
	mSocket.set_option(boost::asio::ip::multicast::join_group(multicastAddress));
}

void PackageReciever::leaveMulticastGroup(const IpAddress& multicastAddress) {
	mSocket.set_option(boost::asio::ip::multicast::leave_group(multicastAddress));
}
