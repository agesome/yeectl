#ifndef YEECTL_DEVICE_MANAGER_WRAPPER_HPP
#define YEECTL_DEVICE_MANAGER_WRAPPER_HPP

#include <QObject>

#include <device_manager.hpp>
#include <device.hpp>

#include <spdlog/spdlog.h>

class device_manager_wrapper : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(int brightness READ get_brightness WRITE set_brightness NOTIFY brightness_changed)
    Q_PROPERTY(bool power READ get_power WRITE set_power NOTIFY power_changed)

    explicit device_manager_wrapper(device_manager & d)
        : _device_manager(d)
    {
        _device_manager.on_current_device_change = [this]()
        {
            brightness_changed(get_brightness());
            power_changed(get_power());

            _device_manager.apply_to_current([&](device & dev)
            {
                dev.on_property_change = [&](std::string_view, std::string_view name)
                {
                    if (name == "bright")
                    {
                        brightness_changed(std::get<int>(dev.get_property("bright")));
                    }
                    else if (name == "power")
                    {
                        power_changed(std::get<std::string>(dev.get_property("power")) == "on");
                    }
                };
            });
        };
    }

    void set_brightness(int v)
    {
        _device_manager.apply_to_current([=](device & d)
        {
            d.set_property("bright", v);
        });
    }

    int get_brightness()
    {
        int v{};
        _device_manager.apply_to_current([&](device & d)
        {
            v = std::get<int>(d.get_property("bright"));
        });
        spdlog::debug("get_brightness: {}", v);
        return v;
    }

    void set_power(bool v)
    {
        _device_manager.apply_to_current([=](device & d)
        {
            d.set_property("power", v ? "on" : "off");
        });
    }

    bool get_power()
    {
        bool power{};
        _device_manager.apply_to_current([&](device & d)
        {
            power = std::get<std::string>(d.get_property("power")) == "on";
        });
        spdlog::debug("get_power: {}", power);
        return power;
    }

signals:
    void brightness_changed(int v);
    void power_changed(bool power);

private:
    device_manager & _device_manager;
};


#endif // YEECTL_DEVICE_MANAGER_WRAPPER_HPP
