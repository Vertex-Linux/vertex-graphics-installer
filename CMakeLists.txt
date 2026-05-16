cmake_minimum_required(VERSION 3.20)
project(arch-gpu-driver-installer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(SOURCES
    src/main.cpp
    src/GpuDetector.cpp
    src/PackageResolver.cpp
    src/Workers.cpp
    src/MainWindow.cpp
)

set(HEADERS
    src/GpuDetector.h
    src/PackageResolver.h
    src/Workers.h
    src/MainWindow.h
)

qt_add_executable(arch-gpu-driver-installer
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(arch-gpu-driver-installer
    PRIVATE
        Qt6::Core
        Qt6::Widgets
)

install(TARGETS arch-gpu-driver-installer
    RUNTIME DESTINATION /usr/local/bin
)
