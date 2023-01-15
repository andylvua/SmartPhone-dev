cmake_minimum_required(VERSION 3.15)
project(phone)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#set(CMAKE_PREFIX_PATH "/home/paul/Qt/5.15.2/gcc_64")
#set(CMAKE_PREFIX_PATH "/Users/andrew/Qt/5.15.2/clang_64")
#set(CMAKE_PREFIX_PATH "/usr/local/qt5pi/")

find_package(Qt5 COMPONENTS
        Core
        Widgets
        SerialPort
        REQUIRED)

add_executable(phone main.cpp Src/command.cpp Inc/command.h Src/serial.cpp Inc/serial.h Inc/modem.h Src/modem.cpp Inc/cli.h Src/cli.cpp Inc/contact.h Src/screen.cpp Inc/screen.h)
target_link_libraries(phone
        Qt5::Core
        Qt5::Widgets
        Qt5::SerialPort
        )


set(CMAKE_PREFIX_PATH "/usr/share/readline")

target_link_libraries(phone readline)