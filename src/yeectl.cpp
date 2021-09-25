#include <QQmlApplicationEngine>
#include <QtGui/QGuiApplication>
#include <QtQuickControls2/QQuickStyle>

#include <thread>

#include <device.hpp>
#include <multicast_worker.hpp>
#include <device_manager.hpp>
#include <device_manager_wrapper.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

int main(int argc, char **argv)
{
    spdlog::set_pattern("[%c] [%t] [%l] %v");
#if 0
    spdlog::set_level(spdlog::level::debug);
#endif

    spdlog::default_logger()->sinks().emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("yeectl.log"));
    // we don't log that much
    spdlog::default_logger()->flush_on(spdlog::level::debug);

    asio::io_context io_context;

    device_manager manager;
    device_manager_wrapper wrapper(manager);
    multicast_worker worker(io_context, manager);

    if (qmlRegisterSingletonInstance("yeectl", 1, 0, "DeviceManager", &wrapper) < 0)
    {
        spdlog::critical("failed to register QML type");
        return EXIT_FAILURE;
    }

    QQuickStyle::setStyle("Material");
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine("qrc:yeectl/src/main.qml");

    std::thread io_thread([&]()
    {
        spdlog::debug("io thread starting");
        try
        {
            io_context.run();
        }
        catch (std::exception & ex)
        {
            spdlog::critical("io thread exception: {}", ex.what());
            app.quit();
        }
    });

    app.exec();
    io_context.stop();
    io_thread.join();

    return EXIT_SUCCESS;
}
