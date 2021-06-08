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

    void process_message(std::string_view view);
    void process_request_result(const nlohmann::json & json);
    void process_property_updates(const nlohmann::json & json);

    asio::ip::tcp::socket   _socket;
    property_map            _properties;
    size_t                  _request_counter;

    std::array<char, kBufferSize>           _buffer;
    std::unordered_map<size_t, std::string> _requests;
    asio::steady_timer                      _timer;
};

#endif // YEECTL_DEVICE_HPP
