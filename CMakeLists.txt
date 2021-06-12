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

qt_add_resources(RESOURCES resources.qrc)
qt_wrap_cpp(MOC_FILES ${QT_HEADER_FILES})

add_executable(yeectl ${SOURCE_FILES} ${RESOURCES} ${MOC_FILES} ${HEADER_FILES} ${QT_HEADER_FILES})
target_include_directories(yeectl PRIVATE src)
target_link_libraries(yeectl Qt::Network Qt::Qml Qt::Gui Qt::QuickControls2)
target_link_libraries(yeectl ws2_32 CONAN_PKG::asio CONAN_PKG::spdlog CONAN_PKG::nlohmann_json)
target_precompile_headers(yeectl PRIVATE ${HEADERS_TO_PRECOMPILE})

if (MSVC)
    target_link_libraries(yeectl ws2_32)
    target_compile_options(yeectl PRIVATE /W4 /MP)
    target_link_options(yeectl PRIVATE /debug:fastlink)
    target_compile_definitions(yeectl PRIVATE _WIN32_WINNT=_WIN32_WINNT_WIN7 _WINSOCK_DEPRECATED_NO_WARNINGS)
endif()
