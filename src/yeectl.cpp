#include <QtNetwork>
#include <QString>
#include <QtGui/QGuiApplication>
#include <QMap>
#include <QRegularExpression>

#include <exception>
#include <functional>

class Device
{
public:
    Device(QStringView discoveryMessage)
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

private:
    QTcpSocket m_socket;

    const QMap<QStringView, std::function<void (QStringView)>> kHandlers = {
        { L"power", [this](QStringView s) {
            qDebug() << "power handler" << s;
            m_isOn = s.startsWith(L"on");
        }},
        { L"bright", [this](QStringView s) {
            m_brightness = s.toShort();
            qDebug() << "bright handler" << s << m_brightness;
        }},
    };

    static inline const QRegularExpression kDiscoveryRegex{"^(\\w+): (\\S*)"};
    static constexpr auto kDiscoveryField = 1;
    static constexpr auto kDiscoveryContent = 2;

    bool m_isOn;
    uint8_t m_brightness;
};

class DeviceList : public QObject
{
public:
    DeviceList()
    {
        connect(&m_socket, &QUdpSocket::readyRead, this, &DeviceList::onDatagram);

        const QString msg = "M-SEARCH * HTTP/1.1\r\n"
                            "ST: wifi_bulb\r\n"
                            "MAN: \"ssdp:discover\"\r\n";

        // bail if either fails
        if (m_socket.bind(QHostAddress::AnyIPv4, kMulticastPort, kSocketFlags) &&
            m_socket.joinMulticastGroup(kMulticastAddress))
        {
            qDebug() << "socket setup done";
            if (auto sz = m_socket.writeDatagram(msg.toUtf8(), kMulticastAddress, kMulticastPort); sz != msg.size())
            {
                qWarning() << "discovery message is" << msg.size() << "bytes, but" << sz << "were sent";
            }
        }
        else
        {
            throw std::runtime_error("socket setup failed");
        }
    }
private:
    void onDatagram()
    {
        qDebug() << "got datagrams";
        while (m_socket.hasPendingDatagrams())
        {
            // steal only the data
            const QString str {std::move(m_socket.receiveDatagram().data())};
            if (str.startsWith("HTTP/1.1 200 OK"))
            {
                std::unique_ptr<Device> d;
                try
                {
                    d = std::make_unique<Device>(str);
                }
                catch (const std::exception & ex)
                {
                    qWarning() << ex.what();
                }
            }
        }
    }

    QUdpSocket m_socket;

    static inline const QHostAddress kMulticastAddress{"239.255.255.250"};
    static constexpr quint16 kMulticastPort = 1982;
    static constexpr auto kSocketFlags = QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress;
};

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    std::unique_ptr<DeviceList> dl;

    try
    {
       dl = std::make_unique<DeviceList>();
    }
    catch (const std::exception & ex)
    {
        qFatal(ex.what());
    }

    return app.exec();
}
