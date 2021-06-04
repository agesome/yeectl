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
    explicit device_manager_wrapper(device_manager & d)
        : _device_manager(d)
    {

    }

    Q_PROPERTY(int brightness READ get_brightness WRITE set_brightness NOTIFY brightness_changed)

    void set_brightness(int v)
    {
        _device_manager.apply_to_current([=](device & d)
        {
            d.set_property("bright", v);
        });

        _device_manager.apply_to_current([&](device & d)
        {
            d.on_property_change = [&](std::string_view id, std::string_view name)
            {
                if (name == "bright")
                {
                    brightness_changed(std::get<int>(d.get_property("bright")));
                }
            };
        });
    }

    int get_brightness()
    {
        int v{};
        _device_manager.apply_to_current([&](device & d)
        {
            v = std::get<int>(d.get_property("bright"));
        });
        spdlog::info("bright: {}", v);
        return v;
    }

signals:
    void brightness_changed(int v);

private:
    device_manager & _device_manager;
};


#endif // YEECTL_DEVICE_MANAGER_WRAPPER_HPP
