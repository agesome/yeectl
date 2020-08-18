#include <device.hpp>

Device::Device(QStringView discoveryMessage)
    : Device()
{
    parseDiscoveryMessage(discoveryMessage);
}

Device::Device()
{
    connect(&m_socket, &QTcpSocket::errorOccurred, this, &Device::onSocketError);
    connect(&m_socket, &QTcpSocket::stateChanged, this, &Device::onSocketStateChange);
    connect(&m_socket, &QTcpSocket::readyRead, this, &Device::onSocketData);
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
}

void Device::parseDiscoveryMessage(QStringView discoveryMessage)
{
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

void Device::onSocketStateChange(QAbstractSocket::SocketState socketState)
{
    qDebug() << "socket state:" << socketState;
    if (socketState == QAbstractSocket::ConnectedState)
    {
        //            toggle();
    }
}
