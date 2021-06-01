#ifndef YEECTL_DEVICE_MANAGER_HPP
#define YEECTL_DEVICE_MANAGER_HPP

#include <device.hpp>

#include <unordered_map>
#include <string_view>

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

    void set_brightness(int v)
    {
        if (!_devices.empty())
        {
            _devices.begin()->second->set_brigtness(v);
        }
    }
private:
    std::unordered_map<std::string_view, std::unique_ptr<device>> _devices;
};

#endif // YEECTL_DEVICE_MANAGER_HPP
