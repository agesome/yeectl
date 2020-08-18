#include <QQmlApplicationEngine>
#include <QtGui/QGuiApplication>

#include <device.hpp>
#include <devicelist.hpp>

int main(int argc, char **argv)
{
    qDebug() << qmlRegisterType<Device>("yeectl", 1, 0, "Device");
    qDebug() << qmlRegisterType<DeviceList>("yeectl", 1, 0, "DeviceList");

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine("qrc:/src/main.qml");

    return app.exec();
}
