#include "packagereceiver.hpp"

using std::vector;
using std::make_unique;
using std::mutex;
using std::unique_lock;

using boost::system::error_code;

PackageReceiver::PackageReceiver(const std::string& multicastAddress, uint16_t port)
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

bool PackageReceiver::start() {
    mIoService.reset();
    doReceive();
    mIoService.run();
    return true;
}

void PackageReceiver::stop() {
    mIoService.stop();
}

void PackageReceiver::setCallback(Callback&& callback) {
    unique_lock<mutex> lock(callbackMutex);
    mCallback = std::move(callback);
}

void PackageReceiver::resetCallback() {
    unique_lock<mutex> lock(callbackMutex);
    mCallback = nullptr;
}

void PackageReceiver::callback(ByteVector& buffer) {
    unique_lock<mutex> lock(callbackMutex);
    if(mCallback)
        mCallback(buffer);
}

void PackageReceiver::doReceive() {
    mBuffer.resize(mBufferSize);
    mSocket.async_receive_from(boost::asio::buffer(mBuffer), mEndpoint,
    [&, this](const error_code & error, size_t size) {
        if(!error) {
            mBuffer.resize(size);
            callback(mBuffer);
        }
        doReceive();
    });
}

void PackageReceiver::joinMulticastGroup(const IpAddress& multicastAddress) {
    mSocket.set_option(boost::asio::ip::multicast::join_group(multicastAddress));
}

void PackageReceiver::leaveMulticastGroup(const IpAddress& multicastAddress) {
    mSocket.set_option(boost::asio::ip::multicast::leave_group(multicastAddress));
}
