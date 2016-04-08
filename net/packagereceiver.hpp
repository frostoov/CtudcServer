#pragma once

#include <boost/asio.hpp>
#include <mutex>

class PackageReceiver {
    using UDP       = boost::asio::ip::udp;
    using Endpoint  = UDP::endpoint;
    using UdpSocket = boost::asio::ip::udp::socket;
    using IoService = boost::asio::io_service;
    using IpAddress = boost::asio::ip::address;
public:
    using Callback  = std::function<void(std::vector<char>&)>;
public:
    PackageReceiver(const std::string& multicastAddress, uint16_t port);
    ~PackageReceiver();
    void start();
    void stop();
    void onRecv(Callback&& callback);
protected:
    void doReceive();
    void joinMulticastGroup(const IpAddress& multicastAddress);
    void leaveMulticastGroup(const IpAddress& multicastAddress);
private:
    IoService mIoService;
    UdpSocket mSocket;
    Endpoint  mEndpoint;
    IpAddress mMulticastAddress;

    std::mutex callbackMutex;
    std::vector<char> mBuffer;
    Callback   mCallback;

    static constexpr size_t mBufferSize = 65527;
};
