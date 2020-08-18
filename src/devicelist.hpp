#ifndef DEVICELIST_HPP
#define DEVICELIST_HPP

#include <QUdpSocket>
#include <unordered_map>

#include <device.hpp>

class DeviceList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Device * currentDevice READ currentDevice)

public:
    DeviceList();

    Device * currentDevice();

private:
    void onDatagram();

    QUdpSocket m_socket;
    std::unordered_map<uint32_t, std::unique_ptr<Device>> m_devices;
};

#endif
