#include "packagereceiver.hpp"

using std::vector;
using std::make_unique;
using std::mutex;
using std::unique_lock;
using std::string;

using boost::system::error_code;

PackageReceiver::PackageReceiver(const string& multicastAddress, uint16_t port)
	: mSocket(mIoService),
	  mEndpoint(UDP::v4(), port),
	  mMulticastAddress(IpAddress::from_string(multicastAddress)),
	  mBuffer(mBufferSize),
	  mCallback(nullptr) {
	mSocket.open(UDP::v4());
	mSocket.bind(mEndpoint);

	joinMulticastGroup(mMulticastAddress);
}

PackageReceiver::~PackageReceiver() {
	stop();
}

void PackageReceiver::start() {
	doReceive();
	mIoService.reset();
	mIoService.run();
}

void PackageReceiver::stop() {
	mIoService.stop();
}

void PackageReceiver::onRecv(Callback&& callback) {
	mCallback = std::move(callback);
}

void PackageReceiver::doReceive() {
	mBuffer.resize(mBufferSize);
	mSocket.async_receive_from(boost::asio::buffer(mBuffer), mEndpoint,
	[&, this](auto& error, size_t size) {
		if(!error) {
			mBuffer.resize(size);
			this->mCallback(mBuffer);
		}
		this->doReceive();
	});
}

void PackageReceiver::joinMulticastGroup(const IpAddress& multicastAddress) {
	mSocket.set_option(boost::asio::ip::multicast::join_group(multicastAddress));
}

void PackageReceiver::leaveMulticastGroup(const IpAddress& multicastAddress) {
	mSocket.set_option(boost::asio::ip::multicast::leave_group(multicastAddress));
}
