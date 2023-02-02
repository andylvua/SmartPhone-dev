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

add_executable(phone main.cpp Src/modem/command/command.cpp Inc/modem/command/command.hpp Src/modem/serial.cpp Inc/modem/serial.hpp Inc/modem/modem.hpp Src/modem/modem.cpp Inc/cli/cli.hpp Src/cli/cli.cpp Src/cli/screen.cpp Inc/cli/screen.hpp Inc/logging.hpp Src/cli/cli.cpp Src/cli/cli.cpp Src/cli/cli.cpp Inc/cli/utils/io/ncurses_io.hpp Src/cli/utils/io/ncurses_io.cpp Inc/rotary_reader/rotary_dial.hpp Src/rotary_reader/rotary_dial.cpp Inc/cli/option.hpp Src/cli/option.cpp Inc/cli/defenitions/colors.hpp Src/modem/utils/decoder.cpp Inc/cli/defenitions/cli_macros.hpp Inc/modem/utils/cache_manager.hpp Src/modem/utils/cache_manager.cpp Inc/modem/media_types.hpp Src/modem/media_types.cpp Src/modem/utils/modem_utils.cpp Src/modem/utils/modem_handlers.cpp Inc/cli/utils/ncurses_utils.hpp Src/cli/utils/ncurses_utils.cpp Src/cli/utils/io/readline_utils.cpp Inc/cli/utils/io/readline_utils.hpp Src/cli/utils/cli_utils.cpp Inc/cli/utils/cli_utils.hpp Src/cli/utils/cli_handlers.cpp Src/cli/utils/consoles.cpp Src/logging.cpp)

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