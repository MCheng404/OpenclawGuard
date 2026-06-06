cmake_minimum_required(VERSION 3.16)
project(OpenclawGuard VERSION 1.9.2 LANGUAGES CXX)

# 构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)

# Debug 版本定义
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(OPENCLAWGUARD_DEBUG=1)
    add_compile_options(-g -O0)
    message(STATUS "构建 Debug 版本（带调试日志）")
else()
    add_compile_definitions(OPENCLAWGUARD_DEBUG=0)
    add_compile_options(-O2)
    message(STATUS "构建 Release 版本")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Svg Xml)

set(SOURCES
    src/main.cpp
    src/config.h
    src/settings.h
    src/settings.cpp
    src/portmonitor.h
    src/portmonitor.cpp
    src/processguard.h
    src/processguard.cpp
    src/gatewaymanager.h
    src/gatewaymanager.cpp
    src/updatemanager.h
    src/updatemanager.cpp
    src/envmanager.h
    src/envmanager.cpp
    src/theme.h
    src/theme.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/toggle_switch.h
    src/toggle_switch.cpp
    src/modernslider.h
    src/modernslider.cpp
    src/liquidglasscard.h
    src/liquidglasscard.cpp
    src/traymanager.h
    src/traymanager.cpp
    src/mica_helper.cpp
)

set(RESOURCES
    resources/resources.qrc
)

add_executable(OpenclawGuard ${SOURCES} ${RESOURCES})

target_include_directories(OpenclawGuard PRIVATE src)

target_link_libraries(OpenclawGuard PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Svg
    Qt6::Xml
)

if(WIN32)
    set_target_properties(OpenclawGuard PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    target_link_libraries(OpenclawGuard PRIVATE dwmapi)
endif()
