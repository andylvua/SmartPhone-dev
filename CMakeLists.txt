cmake_minimum_required(VERSION 3.15)
project(phone)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

#set(CMAKE_PREFIX_PATH "/home/paul/Qt/5.15.2/gcc_64")
#set(CMAKE_PREFIX_PATH "/Users/andrew/Qt/5.15.2/clang_64")
#set(CMAKE_PREFIX_PATH "/usr/local/qt5pi/")

find_package(Qt5 COMPONENTS
        Core
        Widgets
        SerialPort
        REQUIRED)

add_executable(phone main.cpp Src/command/command.cpp Inc/command/command.h Src/modem/serial.cpp Inc/modem/serial.h Inc/modem/modem.h Src/modem/modem.cpp Inc/cli/cli.h Src/cli/cli.cpp Src/cli/screen.cpp Inc/cli/screen.h Inc/logging.h)
target_link_libraries(phone
        Qt5::Core
        Qt5::Widgets
        Qt5::SerialPort
        )


set(CMAKE_PREFIX_PATH "/usr/share")
target_link_libraries(phone readline)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

target_link_libraries(phone spdlog::spdlog)