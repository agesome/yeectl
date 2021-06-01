#include <device.hpp>
#include <util.hpp>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <charconv>

device::device(asio::io_context &context, std::string_view data, property_map properties)
    : _socket(context)
{
    auto endpoint = find_endpoint(std::get<std::string_view>(properties["Location"]));
    _socket.connect(endpoint);

    _raw_data = std::move(data);
    _properties = std::move(properties);

    spdlog::info("device {} connected!", id());

    listen_on_socket();
}

std::string_view device::id()
{
    return std::get<std::string_view>(_properties["id"]);
}

void device::set_brigtness(int v)
{
    static std::string msg;

    msg = { nlohmann::json
    {
        { "id", 0 },
        { "method", "set_bright" },
        { "params", { v, "smooth", 500 } }
    }.dump() + "\r\n"};

    spdlog::info("set_brigtness {} -> {}", v, msg);

    _socket.async_send(asio::buffer(msg), [this](std::error_code error, std::size_t length)
    {
        if (error || length != msg.length())
        {
            spdlog::warn("set_brigtness: sent {} vs {}, error: {}", length, msg.length(), error.message());
        }
        else
        {
            spdlog::info("set_brigtness sent");
        }
    });
}

device::property_map device::parse_multicast(std::string_view view)
{
    property_map properties;

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
//            spdlog::info("{} = {} (str)", key, value);
        }
        else
        {
            properties[key] = iv;
//            spdlog::info("{} = {} (int)", key, iv);
        }
    }

    return properties;
}

asio::ip::tcp::endpoint device::find_endpoint(std::string_view view)
{
    auto line_start = view.find("yeelight://");
    if (line_start == std::string_view::npos)
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

void device::listen_on_socket()
{
    static std::array<char, 1024>   _buffer;

    _socket.async_receive(asio::buffer(_buffer), [this](std::error_code error, std::size_t length)
    {
        if (error)
        {
            spdlog::warn("receive error: {}", error.message());
            return;
        }

        std::string_view view(_buffer.data(), length);
        spdlog::info("from lamp: {}\n{}", length, view);

        listen_on_socket();
    });
}

