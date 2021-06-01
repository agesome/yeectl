#ifndef YEECTL_DEVICE_MANAGER_WRAPPER_HPP
#define YEECTL_DEVICE_MANAGER_WRAPPER_HPP

#include <QObject>
#include <device_manager.hpp>

class device_manager_wrapper : public QObject
{
    Q_OBJECT
public:
    explicit device_manager_wrapper(device_manager & d)
        : _device_manager(d)
    {}

    Q_INVOKABLE void set_brightness(int v)
    {
        _device_manager.set_brightness(v);
    }

private:
    device_manager & _device_manager;
};


#endif // YEECTL_DEVICE_MANAGER_WRAPPER_HPP
