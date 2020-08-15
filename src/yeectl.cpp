#include <QQmlApplicationEngine>
#include <QtGui/QGuiApplication>

#include "yeectl.hpp"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

//    qmlRegisterType<Device>("yeectl", 1, 0, "Device");
    qmlRegisterType<DeviceList>("yeectl", 1, 0, "DeviceList");

//    std::unique_ptr<DeviceList> dl;

//    try
//    {
//       dl = std::make_unique<DeviceList>();
//    }
//    catch (const std::exception & ex)
//    {
//        qFatal(ex.what());
//    }

    QQmlApplicationEngine engine("qrc:///main.qml");

    return app.exec();
}

Device::Device(QStringView discoveryMessage)
{
    connect(&m_socket, &QTcpSocket::errorOccurred, this, &Device::onSocketError);
    connect(&m_socket, &QTcpSocket::stateChanged, this, &Device::onSocketStateChange);
    connect(&m_socket, &QTcpSocket::readyRead, this, &Device::onSocketData);

    const auto l = discoveryMessage.split(L"\r\n");
    for (auto s : l)
    {
        if (auto match = kDiscoveryRegex.match(s); match.hasMatch())
        {
            if (auto field = match.captured(kDiscoveryField); kHandlers.contains(field))
            {
                kHandlers[field](match.captured(kDiscoveryContent));
            }
        }
    }

}

void Device::open()
{
    m_socket.connectToHost(m_address, m_port);
}

void Device::toggle()
{
    QJsonObject object
    {
        {"id", 0},
        {"method", "toggle"},
        {"params", QJsonArray()}
    };
    const auto msg = QJsonDocument(object).toJson(QJsonDocument::Compact) + "\r\n";
    const auto sz = m_socket.write(msg);
    qDebug () << msg << sz << m_socket.bytesToWrite();
    m_socket.flush();
    qDebug() << m_socket.bytesToWrite();
}

void Device::onSocketData()
{
    qDebug() << "data" << m_socket.bytesAvailable();
    auto d = m_socket.readAll();
    qDebug() << d;
}

void Device::onSocketError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "socket error:" << socketError;
}

DeviceList::DeviceList()
{
    connect(&m_socket, &QUdpSocket::readyRead, this, &DeviceList::onDatagram);

    const QString msg = "M-SEARCH * HTTP/1.1\r\n"
                        "HOST: " + kMulticastAddress.toString() + ":" + QString::number(kMulticastPort) + "\r\n"
                        "MAN: \"ssdp:discover\"\r\n"
                        "ST: wifi_bulb\r\n";

    // bail if either fails
    if (m_socket.bind(QHostAddress::AnyIPv4, kMulticastPort, kSocketFlags) &&
            true)
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
