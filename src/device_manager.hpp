#ifndef YEECTL_DEVICE_MANAGER_HPP
#define YEECTL_DEVICE_MANAGER_HPP

#include <device.hpp>

#include <unordered_map>
#include <string_view>
#include <optional>

class device_manager
{
public:
    bool is_known(const std::string & id) const
    {
        return _devices.contains(id);
    }

    void add(std::unique_ptr<device> d)
    {
        // why does this requrie move?
        _devices.emplace(d->id(), std::move(d));
    }

    void apply_to_current(std::function<void (device &)> f)
    {
        if (!_devices.empty())
        {
            f(*_devices.begin()->second);
        }
    }

private:
    std::unordered_map<std::string, std::unique_ptr<device>> _devices;
};

#endif // YEECTL_DEVICE_MANAGER_HPP
