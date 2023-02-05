cmake_minimum_required(VERSION 3.15)
project(phone)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -W -Wall -Wextra")
set(QT_VERSION 5.15.2)

cmake_host_system_information(RESULT _host_name QUERY HOSTNAME)

if(_host_name STREQUAL "raspberrypi")
    set(ON_RASPBERRY ON)
else()
    set(ON_RASPBERRY OFF)
endif()

file(GLOB_RECURSE SOURCES Src/*.cpp)
file(GLOB_RECURSE HEADERS Inc/*.hpp)

add_executable(phone main.cpp ${SOURCES} ${HEADERS} Src/cli/operations/screen_operations.cpp Src/cli/operations/modem_operations.cpp Inc/cli/utils/assertions.hpp Src/cli/utils/assertions.cpp)

set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};$ENV{HOME}/Qt/${QT_VERSION}/gcc_64")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};$ENV{HOME}/Qt/${QT_VERSION}/clang_64")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};/usr/local/qt5pi/")

find_package(Qt5 COMPONENTS
        Core
        Widgets
        SerialPort
        REQUIRED)

if(ON_RASPBERRY)
    target_compile_definitions(phone PUBLIC BUILD_ON_RASPBERRY)

    set(CMAKE_PREFIX_PATH "/usr/local/bin")
    target_link_libraries(phone wiringPi)
endif()

target_link_libraries(phone
        Qt5::Core
        Qt5::Widgets
        Qt5::SerialPort
        )

find_package(PkgConfig REQUIRED)

set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/opt/readline/lib/pkgconfig")
pkg_check_modules(NCURSES REQUIRED IMPORTED_TARGET ncurses)
pkg_check_modules(READLINE REQUIRED IMPORTED_TARGET readline)

target_link_libraries(phone PkgConfig::NCURSES)
target_link_libraries(phone PkgConfig::READLINE)

find_package(spdlog REQUIRED)
target_link_libraries(phone spdlog::spdlog)

target_include_directories(phone PUBLIC Inc)