#ifndef PACKAGERECIEVER_HPP
#define PACKAGERECIEVER_HPP

#include <boost/asio.hpp>
#include <mutex>

#include <trek/data/structs.hpp>

class PackageReceiver {
    using UDP       = boost::asio::ip::udp;
    using Endpoint  = UDP::endpoint;
    using UdpSocket = boost::asio::ip::udp::socket;
    using IoService = boost::asio::io_service;
    using IpAddress = boost::asio::ip::address;
public:
    using ByteVector  = std::vector<char>;
    using Callback  = std::function<void (ByteVector&) >;
public:
    PackageReceiver(const std::string& multicastAddress, uint16_t port);
    PackageReceiver(const std::string& multicastAddress, uint16_t port, const Callback& function);
    ~PackageReceiver();
    bool start();
    void stop();
    void setCallback(Callback&& callback);
    void resetCallback();
protected:
    void callback(ByteVector& buffer);
    void doReceive();
    void joinMulticastGroup(const IpAddress& multicastAddress);
    void leaveMulticastGroup(const IpAddress& multicastAddress);
private:
    IoService mIoService;
    UdpSocket mSocket;
    Endpoint  mEndpoint;
    IpAddress mMulticastAddress;

    std::mutex callbackMutex;
    ByteVector mBuffer;
    Callback   mCallback;

    static const size_t mBufferSize = 65527;
};

#endif // PACKAGERECIEVER_HPP
