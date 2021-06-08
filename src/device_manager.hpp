#ifndef YEECTL_DEVICE_MANAGER_HPP
#define YEECTL_DEVICE_MANAGER_HPP

#include <device.hpp>

#include <unordered_map>
#include <string_view>
#include <optional>
#include <shared_mutex>

class device_manager
{
public:
    std::function<void ()> on_current_device_change;

    bool is_known(const std::string & id)
    {
        std::shared_lock lock(_mutex);
        return _devices.contains(id);
    }

    void add(std::unique_ptr<device> d)
    {
        {
            std::unique_lock lock(_mutex);
            // why does this requrie move?
            _devices.emplace(d->id(), std::move(d));
        }
        if (on_current_device_change)
        {
            on_current_device_change();
        }
    }

    void apply_to_current(std::function<void (device &)> f)
    {
        std::unique_lock lock(_mutex);
        if (!_devices.empty())
        {
            f(*_devices.begin()->second);
        }
    }

private:
    std::unordered_map<std::string, std::unique_ptr<device>> _devices;
    std::shared_mutex _mutex;
};

#endif // YEECTL_DEVICE_MANAGER_HPP
