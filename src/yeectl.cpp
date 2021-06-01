#include <QQmlApplicationEngine>
#include <QtGui/QGuiApplication>

#include <thread>

#include <device.hpp>
#include <multicast_worker.hpp>
#include <device_manager.hpp>
#include <device_manager_wrapper.hpp>

int main(int argc, char **argv)
{
    asio::io_context io_context;

    device_manager manager;
    device_manager_wrapper wrapper(manager);
    multicast_worker worker(io_context, manager);

    std::thread io_thread([&]()
    {
        io_context.run();
    });

    qDebug() << qmlRegisterSingletonInstance("yeectl", 1, 0, "DeviceManager", &wrapper);

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine("qrc:/src/main.qml");

    app.exec();
    io_context.stop();
    return 0;
}
