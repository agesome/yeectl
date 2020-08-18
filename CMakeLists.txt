cmake_minimum_required(VERSION 3.18)

project(yeectl CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Network Gui Qml REQUIRED)

set(SOURCE_FILES
    src/yeectl.cpp
    src/device.cpp
    src/devicelist.cpp
)

set(HEADER_FILES
    src/yeectl.hpp
    src/device.hpp
    src/devicelist.hpp
)

qt_add_resources(RESOURCES resources.qrc)
qt_wrap_cpp(MOC_FILES ${HEADER_FILES})

add_executable(yeectl ${SOURCE_FILES} ${RESOURCES} ${MOC_FILES} ${HEADER_FILES})
target_include_directories(yeectl PRIVATE src)
target_link_libraries(yeectl Qt::Network Qt::Qml Qt::Gui)
