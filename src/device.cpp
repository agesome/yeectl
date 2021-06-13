#include <device.hpp>
#include <util.hpp>

#include <spdlog/spdlog.h>
#include <charconv>
#include <set>

device::device(asio::io_context &context, property_map properties)
    : _socket(context)
    , _timer(context)
    , _properties(std::move(properties))
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
        { "id", 0 },
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

    auto str = std::make_unique<std::string>(json.dump());
    spdlog::info("set {} -> {}", name, *str);
    str->append("\r\n");

    // why do we crash if buffer is constructed directly?
    const auto buf = asio::buffer(*str);
    _socket.async_send(buf, [s = std::move(str), this](std::error_code error, std::size_t length)
    {
        if (error && !try_reconnect(error))
        {
            spdlog::warn("receive error: {}", error.message());
            return;
        }

        if (length != s->length())
        {
            spdlog::warn("set_property: sent {} vs {}", length, s->length());
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
        throw std::runtime_error("no yeelight:// in data");
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
        throw std::runtime_error("from_chars failed");
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

        spdlog::debug("message from lamp received: {}", length);

        std::string_view view(_buffer.data(), length);
        process_messages(util::split_lines(view));
        listen();
    });
}

void device::process_messages(std::vector<std::string_view> lines)
{
    nlohmann::json json;
    property_map updates;

    for (const auto line : lines)
    {
        try
        {
            json = nlohmann::json::parse(line);
        }
        catch (const std::exception & ex)
        {
            spdlog::warn("json::parse failed: {}", ex.what());
            spdlog::warn("message was: {}", line);
            continue;
        }

        if (json.contains("error"))
        {
            spdlog::warn("request failed: {}", json["error"]["message"].front());
        }
        else if (json["method"] == "props")
        {
            util::update_map(updates, extract_property_updates(json));
        }
    }

    util::update_map(_properties, updates);
    notify_property_updates(updates);
}


device::property_map device::extract_property_updates(const nlohmann::json &json)
{
    property_map updates;
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

        updates.insert_or_assign(el.key(), update);
    }

    return updates;
}

void device::notify_property_updates(const property_map &updates)
{
    if (!on_property_change)
    {
        return;
    }

    for (const auto & p : updates)
    {
        spdlog::info("property update: {} = {}", p.first, util::variant_to_string(p.second));
        on_property_change(id(), p.first);
    }
}

void device::connect()
{
    const auto endpoint = find_endpoint(std::get<std::string>(_properties["Location"]));

    _socket.async_connect(endpoint, [endpoint, this](std::error_code error)
    {
        if (!error)
        {
            spdlog::info("device at {} connected", endpoint.address().to_string());
            notify_property_updates(_properties);
        }
        else
        {
            spdlog::warn("failed to connect: {}", error.message());
            _timer.expires_after(kReconnectTimeout);
            _timer.async_wait([this](std::error_code error)
            {
                if (error)
                {
                    spdlog::error("timer wait failed: {}", error.message());
                    return;
                }
                connect();
            });
        }
    });
}

bool device::try_reconnect(std::error_code error)
{
    const bool is_reconnectible = (asio::error::eof == error) || (asio::error::connection_reset == error);
    if (is_reconnectible)
    {
        spdlog::info("try to reconnect: {}", error.message());
        _socket.close();
        connect();
        listen();
    }
    return is_reconnectible;
}
