#include <multicast_worker.hpp>
#include <device.hpp>
#include <device_manager.hpp>

const asio::ip::address_v4 multicast_worker::kMulticastAddress = asio::ip::make_address_v4("239.255.255.250");

multicast_worker::multicast_worker(asio::io_context &context, device_manager &manager)
    : _socket(context)
    , _timer(context)
    , _resolver(context)
    , _device_manager(manager)
{
    spdlog::info("starting on {}:{}", kMulticastAddress.to_string(), kMulticastPort);
    try
    {
        _socket.open(asio::ip::udp::v4());
        _socket.set_option(asio::ip::multicast::join_group(kMulticastAddress));
        _socket.set_option(asio::ip::udp::socket::reuse_address(true));
        _socket.set_option(asio::socket_base::broadcast(true));
        _socket.set_option(asio::ip::multicast::enable_loopback(false));

        _resolver.async_resolve(asio::ip::udp::v4(), asio::ip::host_name(), std::string(),
                               [&](std::error_code error, auto result)
        {
            if (error)
            {
                throw std::exception(fmt::format("failed to resolve local hostname: {}", error.message()).c_str());
            }

            spdlog::info("resolved local ip: {} -> {}", asio::ip::host_name(), result->endpoint().address().to_string());
            _socket.bind(result->endpoint());
            do_receive(context);
            do_send();
        });
    }
    catch (std::exception & ex)
    {
        spdlog::critical("{}", ex.what());
    }
}

void multicast_worker::do_receive(asio::io_context &context)
{
    _socket.async_receive(asio::buffer(_buffer), [this, &context](std::error_code error, std::size_t length)
    {
        if (error)
        {
            spdlog::warn("receive error: {}", error.message());
            return;
        }

        const std::string_view view(_buffer.data(), length);
        const auto properties = device::parse_multicast(view);
        if (properties.contains("Location") && !_device_manager.is_known(std::get<std::string>(properties.at("id"))))
        {
            _device_manager.add(std::make_unique<device>(context, properties));
        }

        do_receive(context);
    });
}

void multicast_worker::do_send()
{
    static const auto search_message = fmt::format(
                "M-SEARCH * HTTP/1.1\r\n"
                "HOST: {}:{}\r\n"
                "MAN: \"ssdp:discover\"\r\n"
                "ST: wifi_bulb",
                kMulticastAddress.to_string(), kMulticastPort);

    static const asio::ip::udp::endpoint send_endpoint(kMulticastAddress, kMulticastPort);

    _socket.async_send_to(asio::buffer(search_message), send_endpoint,
                          [this](std::error_code error, std::size_t length)
    {
        if (error || length != search_message.length())
        {
            spdlog::warn("multicast: sent {} vs {}, error: {}", length, search_message.length(), error.message());
        }
        else
        {
            spdlog::debug("multicast sent");
        }

        _timer.expires_after(kMulticastInterval);
        _timer.async_wait([this](std::error_code error)
        {
            if (error)
            {
                spdlog::error("timer wait failed: {}", error.message());
            }
            else
            {
                do_send();
            }
        });
    });
}
