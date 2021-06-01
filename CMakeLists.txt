cmake_minimum_required(VERSION 3.19)

project(yeectl CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(Qt6 COMPONENTS Network Gui Qml QuickControls2 REQUIRED)

set(SOURCE_FILES
    src/yeectl.cpp
    src/multicast_worker.cpp
    src/device.cpp
)

set(HEADER_FILES
    src/util.hpp
    src/multicast_worker.hpp
    src/device_manager.hpp
    src/device_manager_wrapper.hpp
    src/device.hpp
)

qt_add_resources(RESOURCES resources.qrc)
qt_wrap_cpp(MOC_FILES ${HEADER_FILES})

add_executable(yeectl ${SOURCE_FILES} ${RESOURCES} ${MOC_FILES} ${HEADER_FILES})
target_include_directories(yeectl PRIVATE src)
target_link_libraries(yeectl Qt::Network Qt::Qml Qt::Gui Qt::QuickControls2)
target_link_libraries(yeectl ws2_32 CONAN_PKG::asio CONAN_PKG::spdlog CONAN_PKG::nlohmann_json)
