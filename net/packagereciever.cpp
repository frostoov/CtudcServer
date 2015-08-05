#include "packagereciever.hpp"


using std::vector;
using std::thread;
using std::make_unique;

using boost::system::error_code;

PackageReceiver::PackageReceiver(const std::string& multicastAddress, uint16_t port)
	: mSocket(mIoService),
	  mEndpoint(UDP::v4(), port),
	  mMulticastAddress(IpAddress::from_string(multicastAddress)),
	  mBuffer(mBufferSize) {
	mSocket.open(UDP::v4());
	mSocket.bind(mEndpoint);

	joinMulticastGroup(mMulticastAddress);
	mSocket.set_option(UDP::socket::receive_buffer_size(mBuffer.size()));
	mIsActive = false;
}

PackageReceiver::~PackageReceiver() {
	stop();
}

bool PackageReceiver::start() {
	if(mIsActive == false) {
		mIoService.reset();
		mIsActive = true;
		doReceive();

		mThread = make_unique<thread>([this]() {mIoService.run(); });
		return true;
	}
	return false;
}

void PackageReceiver::stop() {
	if(mIsActive == true) {
		mIsActive = false;
		mDataChannel.close();
		mIoService.stop();
		mThread->join();
		mThread.reset();
	}
}

bool PackageReceiver::isActive() const {
	return mIsActive;
}

PackageReceiver::DataChannel& PackageReceiver::getDataChannel() {
	return mDataChannel;
}

void PackageReceiver::doReceive() {
	mSocket.async_receive_from(boost::asio::buffer(mBuffer), mEndpoint,
	[&, this](const error_code & error, size_t size) {
		if(!error && mIsActive) {
			mBuffer.resize(size);
			if(mDataChannel.isOpen()) {
				mDataChannel.send(mBuffer);
			}
		}
		if(mIsActive && mDataChannel.isOpen()) {
			mBuffer.resize(mBufferSize);
			doReceive();
		}
	});
}

void PackageReceiver::joinMulticastGroup(const IpAddress& multicastAddress) {
	mSocket.set_option(boost::asio::ip::multicast::join_group(multicastAddress));
}

void PackageReceiver::leaveMulticastGroup(const IpAddress& multicastAddress) {
	mSocket.set_option(boost::asio::ip::multicast::leave_group(multicastAddress));
}
