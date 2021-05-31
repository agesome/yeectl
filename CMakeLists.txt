cmake_minimum_required(VERSION 3.18)

project(yeectl CXX)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_AUTOMOC ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

#find_package(Qt6 COMPONENTS Network Gui Qml REQUIRED)

set(SOURCE_FILES
    src/yeectl.cpp
    src/util.cpp
#    src/device.cpp
#    src/devicelist.cpp
)

set(HEADER_FILES
    src/util.hpp
#    src/yeectl.hpp
#    src/device.hpp
#    src/devicelist.hpp
)

#qt_add_resources(RESOURCES resources.qrc)
#qt_wrap_cpp(MOC_FILES ${HEADER_FILES})

add_executable(yeectl ${SOURCE_FILES} ${RESOURCES} ${MOC_FILES} ${HEADER_FILES})
target_include_directories(yeectl PRIVATE src)
#target_link_libraries(yeectl Qt::Network Qt::Qml Qt::Gui)
target_link_libraries(yeectl ws2_32 CONAN_PKG::asio CONAN_PKG::spdlog CONAN_PKG::nlohmann_json)
