#ifndef YEECTL_DEVICE_HPP
#define YEECTL_DEVICE_HPP

#include <unordered_map>
#include <asio.hpp>
#include <variant>

class device
{
public:
    using property_map = std::unordered_map<std::string_view, std::variant<std::string_view, int>>;

    device(asio::io_context & context, std::string_view data, property_map properties);

    std::string_view id();
    void set_brigtness(int v);

    static property_map parse_multicast(std::string_view view);
private:
    static asio::ip::tcp::endpoint find_endpoint(std::string_view view);
    void listen_on_socket();

    asio::ip::tcp::socket   _socket;
    property_map            _properties;
    std::string             _raw_data;
};

#endif // YEECTL_DEVICE_HPP
