#include <devicelist.hpp>
#include <device.hpp>

#include <QRegularExpression>

#include <exception>

DeviceList::DeviceList()
{
    connect(&m_socket, &QUdpSocket::readyRead, this, &DeviceList::onDatagram);

    const QHostAddress kMulticastAddress{"239.255.255.250"};
    constexpr quint16 kMulticastPort = 1982;
    constexpr auto kSocketFlags = QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress;

    const QString msg = "M-SEARCH * HTTP/1.1\r\n"
                        "HOST: " + kMulticastAddress.toString() + ":" + QString::number(kMulticastPort) + "\r\n"
                        "MAN: \"ssdp:discover\"\r\n"
                        "ST: wifi_bulb\r\n";

    // bail if either fails
    if (m_socket.bind(QHostAddress::AnyIPv4, kMulticastPort, kSocketFlags) &&
        m_socket.joinMulticastGroup(kMulticastAddress))
    {
        qDebug() << "socket setup done";
        qDebug () << msg;
        if (auto sz = m_socket.writeDatagram(msg.toUtf8(), kMulticastAddress, kMulticastPort); sz != msg.size())
        {
            qWarning() << "discovery message is" << msg.size() << "bytes, but" << sz << "were sent";
        }
        qDebug() << "discovery message sent";
    }
    else
    {
        throw std::runtime_error("socket setup failed");
    }
}

Device *DeviceList::currentDevice()
{
    return m_devices.begin()->second.get();
}

void DeviceList::onDatagram()
{
    qDebug() << "got datagrams";
    while (m_socket.hasPendingDatagrams())
    {
        const QString str {std::move(m_socket.receiveDatagram().data())};
        if (str.startsWith("HTTP/1.1 200 OK"))
        {
            qDebug() << str;
            std::unique_ptr<Device> d;
            try
            {
                d = std::make_unique<Device>(str);
            }
            catch (const std::exception & ex)
            {
                qWarning() << ex.what();
            }

            if (d && m_devices.find(d->id()) == m_devices.end())
            {
                d->open();
                m_devices[d->id()] = std::move(d);
            }
        }
    }
}
