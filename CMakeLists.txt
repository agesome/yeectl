cmake_minimum_required(VERSION 3.19)
project(yeectl CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

find_package(Qt6 COMPONENTS Core Gui QuickControls2 REQUIRED)
qt_standard_project_setup()

find_package(asio REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

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

qt_add_executable(yeectl ${SOURCE_FILES} ${RESOURCES}  ${HEADER_FILES} ${QT_HEADER_FILES})

if (CMAKE_BUILD_TYPE STREQUAL "Release" AND WIN32)
    set_target_properties(yeectl PROPERTIES WIN32_EXECUTABLE TRUE)
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
target_link_libraries(yeectl PRIVATE asio::asio spdlog::spdlog nlohmann_json::nlohmann_json)
target_precompile_headers(yeectl PRIVATE ${HEADERS_TO_PRECOMPILE})

if (MSVC)
    target_compile_options(yeectl PRIVATE /W4 /MP)
    target_link_options(yeectl PRIVATE /debug:fastlink)
    target_compile_definitions(yeectl PRIVATE _WIN32_WINNT=_WIN32_WINNT_WIN10)
endif()


install(TARGETS yeectl
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET yeectl
    FILENAME_VARIABLE deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})

qt_finalize_project()
