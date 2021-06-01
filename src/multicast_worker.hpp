#ifndef YEECTL_MULTICAST_WORKER_HPP
#define YEECTL_MULTICAST_WORKER_HPP

#include <spdlog/spdlog.h>
#include <asio.hpp>
#include <chrono>

class device_manager;

class multicast_worker
{
public:
    multicast_worker(asio::io_context & context,
                     device_manager & manager);

    void do_receive(asio::io_context & context);

    void do_send();

private:
    static const asio::ip::address_v4 kMulticastAddress;
    static constexpr auto kMulticastPort        = 1982U;
    static constexpr auto kMulticastInterval    = std::chrono::seconds(10);
    static constexpr auto kBufferSize           = 2048U;

    asio::ip::udp::socket           _socket;
    asio::steady_timer              _timer;
    std::array<char, kBufferSize>   _buffer;

    device_manager & _device_manager;
};



#endif // YEECTL_MULTICAST_WORKER_HPP
