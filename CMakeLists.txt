cmake_minimum_required(VERSION 3.18)

project(yeectl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Network Gui Qml REQUIRED)

set(SOURCE_FILES
    src/yeectl.cpp)

add_executable(yeectl ${SOURCE_FILES})
target_link_libraries(yeectl Qt::Network Qt::Qml Qt::Gui)
