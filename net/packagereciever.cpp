#include "packagereciever.hpp"

using std::vector;
using std::thread;
using std::make_unique;

using boost::system::error_code;

PackageReciever::PackageReciever(const std::string& multicastAddress, uint16_t port)
	: mSocket(mIoService),
	  mEndpoint(UDP::v4(), port),
	  mMulticastAddress(IpAddress::from_string(multicastAddress)),
	  mBuffer(mBufferSize) {
	mSocket.open(UDP::v4());
	mSocket.bind(mEndpoint);

	mSocket.set_option(UDP::socket::receive_buffer_size(mBuffer.size()));
	mIsActive = false;
}

PackageReciever::~PackageReciever() {
	stop();
}

bool PackageReciever::start() {
	if(mIsActive == false) {
		mIsActive = true;
		joinMulticastGroup(mMulticastAddress);
		doReceive();

		mThread = make_unique<thread>([this]() {mIoService.run(); });
		return true;
	}
	return false;
}

void PackageReciever::stop() {
	if(mIsActive == true) {
		mIsActive = false;
		mStopChannel.send(true);
		mIoService.stop();
		leaveMulticastGroup(mMulticastAddress);
		mIoService.reset();
		mThread->join();
		mThread.reset();
	}
}

bool PackageReciever::isActive() const {
	return mIsActive;
}

PackageReciever::DataChannel PackageReciever::getDataChannel() const {
	return mDataChannel;
}

void PackageReciever::doReceive() {
	mSocket.async_receive_from(boost::asio::buffer(mBuffer), mEndpoint,
	[&, this](const error_code & error, size_t size) {
		if(!error && mIsActive) {
			cpp::select select;
			mBuffer.resize(size);
			select.send_only(mDataChannel, mBuffer);
			select.recv(mStopChannel, [this](bool) {});
			select.wait();
		}
		if(!mIsActive) {
			mBuffer.resize(mBufferSize);
			doReceive();
		}
	});
}

void PackageReciever::joinMulticastGroup(const IpAddress& multicastAddress) {
	mSocket.set_option(boost::asio::ip::multicast::join_group(multicastAddress));
}

void PackageReciever::leaveMulticastGroup(const IpAddress& multicastAddress) {
	mSocket.set_option(boost::asio::ip::multicast::leave_group(multicastAddress));
}
