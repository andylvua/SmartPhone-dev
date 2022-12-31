cmake_minimum_required(VERSION 3.24)
project(untitled3)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#set(CMAKE_PREFIX_PATH "/home/paul/Qt/5.15.2/gcc_64")
#set(CMAKE_PREFIX_PATH "/Users/andrew/Qt/5.15.2/clang_64")


find_package(Qt5 COMPONENTS
        Core
        Widgets
        SerialPort
        REQUIRED)

add_executable(untitled3 main.cpp Src/command.cpp Inc/command.h Src/serial.cpp Inc/serial.h Inc/modem.h Src/modem.cpp)
target_link_libraries(untitled3
        Qt5::Core
        Qt5::Widgets
        Qt5::SerialPort
        )

