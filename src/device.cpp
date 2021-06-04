#include <device.hpp>
#include <util.hpp>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <charconv>
#include <set>

device::device(asio::io_context &context, property_map properties)
    : _socket(context)
    , _properties(std::move(properties))
    , _request_counter(0)
{
    connect();
    listen();
}

const std::string &device::id() const
{
    return std::get<std::string>(_properties.at("id"));
}

const device::property & device::get_property(const std::string & name)
{
    return _properties[name];
}

void device::set_property(const std::string &name, const property &value)
{
    auto json = nlohmann::json
    {
        { "id", _request_counter },
        { "method", fmt::format("set_{}", name) },
    };

    if (std::holds_alternative<std::string>(value))
    {
        json["params"] = { std::get<std::string>(value) };
    }
    else
    {
        json["params"] = { std::get<int>(value) };
    }

    auto json_str = json.dump();
    spdlog::info("set {} -> {}", name, json_str);
    const auto result = _requests.emplace(_request_counter++, std::string({ std::move(json_str) + "\r\n" }));
    const auto & str = result.first->second;

    _socket.async_send(asio::buffer(str), [=](std::error_code error, std::size_t length)
    {
        if (error && !try_reconnect(error))
        {
            spdlog::warn("receive error: {}", error.message());
            return;
        }

        if (length != str.length())
        {
            spdlog::warn("set_property: sent {} vs {}", length, str.length());
            _requests.erase(result.first);
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

        int intvalue{};
        property prop;
        static const std::set<std::string_view> force_string_type{ "id", "power" };
        if (auto [_, error] = std::from_chars(value.data(), &value.data()[value.size()], intvalue);
                error == std::errc::invalid_argument || force_string_type.contains(key))
        {
            prop = std::string(value);
        }
        else
        {
            prop = intvalue;
        }

        properties.emplace(std::string(key), std::move(prop));
    }

    return properties;
}

asio::ip::tcp::endpoint device::find_endpoint(const std::string & view)
{
    auto line_start = view.find("yeelight://");
    if (line_start == std::string::npos)
    {
        throw std::exception("no yeelight:// in data");
    }

    auto line_end = view.find_first_of('\n', line_start);
    auto line = view.substr(line_start, line_end - line_start);
    auto pair = line.substr(line.find_last_of('/') + 1);
    auto colon = pair.find_first_of(':');

    auto port_str = pair.substr(colon + 1);
    auto ip_str = pair.substr(0, colon);
    spdlog::info("found endpoint: {}:{}", ip_str, port_str);

    unsigned short port{};
    if (auto [_, error] = std::from_chars(port_str.data(), &port_str.data()[port_str.size()], port);
            error == std::errc::invalid_argument)
    {
        throw std::exception("from_chars failed");
    }

    return { asio::ip::make_address_v4(ip_str), port };
}

void device::listen()
{
    _socket.async_receive(asio::buffer(_buffer), [this](std::error_code error, std::size_t length)
    {
        if (error && !try_reconnect(error))
        {
            spdlog::warn("receive error: {}", error.message());
            return;
        }

        std::string_view view(_buffer.data(), length);
        nlohmann::json json;
        try
        {
            json = nlohmann::json::parse(view);
        }
        catch (const std::exception & ex)
        {
            spdlog::warn("json::parse failed: {}", ex.what());
            spdlog::warn("message was: {}", view);
            listen();
        }

        if (json.contains("id"))
        {
            const auto id = json.at("id").get<size_t>();
            _requests.erase(id);
            spdlog::info("request {} completed: {}", id, json["result"].front());
        }
        else if (json["method"] == "props")
        {
           for (auto & el : json["params"].items())
           {
               property update;
               if (el.value().is_number())
               {
                   update = el.value().get<int>();
               }
               else
               {
                   update = el.value().get<std::string>();
               }

               spdlog::info("property update: {} = {}", el.key(), el.value().dump());
               const auto v = _properties.insert_or_assign(el.key(), update);

               if (on_property_change)
               {
                   on_property_change(id(), v.first->first);
               }
           }
        }

        listen();
    });
}

void device::connect()
{
    auto endpoint = find_endpoint(std::get<std::string>(_properties["Location"]));
    _socket.connect(endpoint);
    spdlog::info("device at {} connected", endpoint.address().to_string());
    if (on_property_change)
    {
        for (const auto & p : _properties)
        {
            on_property_change(id(), p.first);
        }
    }
}

bool device::try_reconnect(std::error_code error)
{
    const bool is_reconnectible = (asio::error::eof == error) || (asio::error::connection_reset == error);
    if (is_reconnectible)
    {
        spdlog::info("try to reconnect: {}", error.message());
        _socket.close();
        _request_counter = 0;
        _requests.clear();
        connect();
        listen();
    }
    return is_reconnectible;
}
