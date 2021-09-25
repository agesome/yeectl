cmake_minimum_required(VERSION 3.19)

project(yeectl CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

set(SOURCE_FILES
    src/yeectl.cpp
    src/multicast_worker.cpp
    src/device.cpp
    src/util.cpp
)

set(HEADER_FILES
    src/util.hpp
    src/multicast_worker.hpp
    src/device_manager.hpp
    src/device.hpp
)

set(QT_HEADER_FILES
    src/device_manager_wrapper.hpp
)

set(RESOURCES
    icons/lamp.svg
)

set(HEADERS_TO_PRECOMPILE
    src/util.hpp

    <spdlog/spdlog.h>
    <asio.hpp>
    <nlohmann/json.hpp>

    <variant>
    <unordered_map>
    <string_view>
    <string>
    <set>
    <optional>
)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(Qt6 COMPONENTS Gui Qml QuickControls2 REQUIRED)

if (CMAKE_BUILD_TYPE STREQUAL "Release" AND WIN32)
    add_executable(yeectl WIN32 ${SOURCE_FILES} ${RESOURCES} ${HEADER_FILES} ${QT_HEADER_FILES})
else()
    add_executable(yeectl ${SOURCE_FILES} ${RESOURCES}  ${HEADER_FILES} ${QT_HEADER_FILES})
endif()

qt_add_qml_module(yeectl
    URI yeectl
    VERSION 1.0
    QML_FILES src/main.qml
)

qt_add_resources(yeectl resources
    FILES ${RESOURCES}
    PREFIX /
)

target_include_directories(yeectl PRIVATE src)
target_link_libraries(yeectl PRIVATE Qt::Qml Qt::Gui Qt::QuickControls2)
target_link_libraries(yeectl PRIVATE ws2_32 CONAN_PKG::asio CONAN_PKG::spdlog CONAN_PKG::nlohmann_json)
target_precompile_headers(yeectl PRIVATE ${HEADERS_TO_PRECOMPILE})

if (MSVC)
    target_link_libraries(yeectl PRIVATE ws2_32)
    target_compile_options(yeectl PRIVATE /W4 /MP)
    target_link_options(yeectl PRIVATE /debug:fastlink)
    target_compile_definitions(yeectl PRIVATE _WIN32_WINNT=_WIN32_WINNT_WIN7 _WINSOCK_DEPRECATED_NO_WARNINGS)
endif()
