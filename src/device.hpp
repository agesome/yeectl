#ifndef YEECTL_DEVICE_HPP
#define YEECTL_DEVICE_HPP

#include <unordered_map>
#include <asio.hpp>
#include <variant>
#include <nlohmann/json.hpp>

class device
{
public:
    using property = std::variant<std::string, int>;
    using property_map = std::unordered_map<std::string, property>;

    device(asio::io_context & context, property_map properties);

    const std::string & id() const;
    const property & get_property(const std::string & name);
    void set_property(const std::string & name, const property & v);

    static property_map parse_multicast(std::string_view view);

    std::function<void (std::string_view id, std::string_view name)> on_property_change;

private:
    static constexpr auto kBufferSize = 1024U;
    static constexpr auto kReconnectTimeout = std::chrono::seconds(5);

    static asio::ip::tcp::endpoint find_endpoint(const std::string &view);
    void listen();
    void connect();
    bool try_reconnect(std::error_code error);

    void process_messages(std::vector<std::string_view> lines);
    property_map extract_property_updates(const nlohmann::json & json);

    void notify_property_updates(const property_map & updates);

    asio::ip::tcp::socket   _socket;
    asio::steady_timer      _timer;
    property_map            _properties;

    std::array<char, kBufferSize> _buffer;

};

#endif // YEECTL_DEVICE_HPP
