#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <QtNetwork>
#include <QString>
#include <QRegularExpression>

#include <exception>
#include <functional>
#include <map>

class Device : public QObject
{
    Q_OBJECT
public:
    Device(QStringView discoveryMessage);
    Device();

    uint32_t id() const
    {
        return m_id;
    }

    void open();

public slots:
    void toggle();

private:
    void parseDiscoveryMessage(QStringView discoveryMessage);

    void onSocketData();

    void onSocketError(QAbstractSocket::SocketError socketError);

    void onSocketStateChange(QAbstractSocket::SocketState socketState);

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
        { L"id", [this](QStringView s) {
            bool ok = false;
            m_id = s.toULongLong(&ok, 16);
            qDebug() << "id:" << s << ok << m_id;
        }},
        { L"Location", [this](QStringView s) {
            QRegularExpression regex {"yeelight:\\/\\/(.+):(.+)"};
            constexpr auto kLocationIp = 1;
            constexpr auto kLocationPort = 2;

            if (auto match = regex.match(s); match.hasMatch() && match.lastCapturedIndex() >= kLocationPort)
            {
                qDebug() << "location handler" << match.captured(kLocationIp) << match.captured(kLocationPort);
                m_address = QHostAddress(match.captured(kLocationIp));
                m_port = match.captured(kLocationPort).toUInt();
            }
        }},
    };

    static inline const QRegularExpression kDiscoveryRegex {"^(\\w+): (\\S*)"};
    static constexpr auto kDiscoveryField = 1;
    static constexpr auto kDiscoveryContent = 2;

    bool m_isOn;
    uint8_t m_brightness;
    uint32_t m_id;

    QHostAddress m_address;
    quint16 m_port;
};

#endif
