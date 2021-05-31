//#include <QQmlApplicationEngine>
//#include <QtGui/QGuiApplication>

//#include <device.hpp>
//#include <devicelist.hpp>

#define  _SILENCE_CLANG_COROUTINE_MESSAGE 1

#include "util.hpp"

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <asio.hpp>

#include <map>
#include <functional>
#include <charconv>
#include <optional>
#include <variant>

class device
{
public:
    using property_map = std::map<std::string_view, std::variant<std::string_view, int>>;

    device(asio::io_context & context, std::string_view data, property_map properties)
        : _socket(context)
    {
        auto endpoint = find_endpoint(std::get<std::string_view>(properties["Location"]));
        _socket.connect(endpoint);

        _raw_data = std::move(data);
        _properties = std::move(properties);

        spdlog::info("device {} connected!", id());
    }

    auto id() -> std::string_view
    {
        return std::get<std::string_view>(_properties["id"]);
    }

    static auto parse_multicast(std::string_view view)
    {
        std::map<std::string_view, std::variant<std::string_view, int>> properties;

        const auto lines = util::split_lines(view);
        for (const auto & l : lines)
        {
            auto maybe_kv = util::split_key_and_value(l);
            if (!maybe_kv.has_value())
            {
                continue;
            }

            auto [key, value] = maybe_kv.value();

            int iv{};
            if (auto [_, error] = std::from_chars(value.data(), &value.data()[value.size()], iv);
                    error == std::errc::invalid_argument || key == "id")
            {
                properties[key] = value;
                spdlog::info("{} = {} (str)", key, value);
            }
            else
            {
                properties[key] = iv;
                spdlog::info("{} = {} (int)", key, iv);
            }


        }

        return properties;
    }

private:
    static asio::ip::tcp::endpoint find_endpoint(std::string_view view)
    {
        auto line_start = view.find("yeelight://");
        if (line_start == view.npos)
        {
            throw std::exception("no yeelight:// in data");
        }

        auto line_end = view.find_first_of('\n', line_start);
        auto line = view.substr(line_start, line_end - line_start);
        auto pair = line.substr(line.find_last_of('/') + 1);
        auto colon = pair.find_first_of(':');

        auto port_str = pair.substr(colon + 1);
        auto ip_str = pair.substr(0, colon);
        spdlog::info("found endpoint: ip:port -> {} : {}", ip_str, port_str);

        unsigned short port{};
        if (auto [_, error] = std::from_chars(port_str.data(), &port_str.data()[port_str.size()], port);
                error == std::errc::invalid_argument)
        {
            throw std::exception("from_chars failed");
        }

        return { asio::ip::make_address_v4(ip_str), port };
    }

    asio::ip::address_v4 _address;
    asio::ip::tcp::socket _socket;

    property_map _properties;
    std::string _raw_data;
};

class device_manager
{
public:
    bool is_known(std::string_view id)
    {
        return _devices.contains(id);
    }

    void add(std::unique_ptr<device> d)
    {
        // why does this requrie move?
        _devices.emplace(d->id(), std::move(d));
    }
private:
    std::map<std::string_view, std::unique_ptr<device>> _devices;
};

class multicast_worker
{
public:
    multicast_worker(asio::io_context & context,
                     device_manager & manager)
        : _socket(context)
        , _timer(context)
        , _device_manager(manager)
    {
        spdlog::info("starting on {}:{}", kMulticastAddress.to_string(), kMulticastPort);
        asio::ip::udp::endpoint listen_endpoint (asio::ip::address_v4(), kMulticastPort);
        try
        {
            _socket.open(listen_endpoint.protocol());
            _socket.set_option(asio::ip::multicast::join_group(kMulticastAddress));
            _socket.set_option(asio::ip::udp::socket::reuse_address(true));
            _socket.set_option(asio::socket_base::broadcast(true));
            _socket.bind(listen_endpoint);
        }
        catch (std::exception & ex)
        {
            spdlog::critical("{}", ex.what());
        }

        do_receive(context);
        do_send();
    }

    void do_receive(asio::io_context & context)
    {
        _socket.async_receive(asio::buffer(_buffer), [this, &context](std::error_code error, std::size_t length)
        {
            if (error)
            {
                spdlog::warn("receive error: {}", error.message());
                return;
            }

            std::string_view view(_buffer.data(), length);
            spdlog::info("len: {}\n{}", length, view);

            auto properties = device::parse_multicast(view);
            if (properties.contains("Location") && !_device_manager.is_known(std::get<std::string_view>(properties["id"])))
            {
                _device_manager.add(std::make_unique<device>(context, view, properties));
            }

            do_receive(context);
        });
    }

    void do_send()
    {
        static const auto search_message = fmt::format(
                "M-SEARCH * HTTP/1.1\r\n"
                "HOST: {}:{}\r\n"
                "MAN: \"ssdp:discover\"\r\n"
                "ST: wifi_bulb\r\n",
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
                spdlog::info("multicast sent");
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

const asio::ip::address_v4 multicast_worker::kMulticastAddress = asio::ip::make_address_v4("239.255.255.250");

int main(int argc, char **argv)
{
    asio::io_context io_context;
    device_manager manager;
    multicast_worker worker(io_context, manager);
    io_context.run();

//    qDebug() << qmlRegisterType<Device>("yeectl", 1, 0, "Device");
//    qDebug() << qmlRegisterType<DeviceList>("yeectl", 1, 0, "DeviceList");

//    QGuiApplication app(argc, argv);
//    QQmlApplicationEngine engine("qrc:/src/main.qml");

//    return app.exec();
    return 0;
}
