#include <QApplication>
#include <QtSerialPort/QSerialPort>
#include "logging.hpp"
#include "modem/serial.hpp"
#include "modem/utils/cache_manager.hpp"
#include "cli/cli.hpp"
#include "cli/modem_controller.hpp"
#include "cli/definitions/colors.hpp"
#include <thread>

//#define DEBUG

int main(int argc, char *argv[]) {
    qInstallMessageHandler(logOutputHandler);
    QApplication app(argc, argv);

    auto mainLogger = spdlog::basic_logger_mt("main",
                                              LOGS_FILEPATH,
                                              true);

    spdlog::flush_on(spdlog::level::debug);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] [function %!] [line %#] %v");

    #ifdef __APPLE__
        const char* portName = "/dev/tty.usbserial-1410";
    #elif __linux__
        const char* portName = "/dev/ttyUSB0";
    #endif

    SerialPort serial(portName,
                      500,
                      QSerialPort::Baud115200,
                      QSerialPort::Data8,
                      QSerialPort::NoParity,
                      QSerialPort::OneStop,
                      QSerialPort::NoFlowControl);

    #ifndef DEBUG
    if (!serial.openSerialPort()) {
        std::cout << RED_COLOR << "Serial port error: " << serial.errorString().toStdString() <<
        ". Check module connection" << RESET << std::endl;
        SPDLOG_LOGGER_ERROR(mainLogger, "Serial port was not opened: \"{}\"", serial.errorString().toStdString());
        return 1;
    } else {
        SPDLOG_LOGGER_INFO(mainLogger, "Serial port was opened successfully. Starting modem");
    }
    #endif


    Modem modem{serial};
    CLI cli;

    ModemController modemController{cli, modem};
    cli.setModemController(&modemController);

    #ifndef DEBUG
    bool modemReady = modem.initialize();

    if (!modemReady) {
        SPDLOG_LOGGER_ERROR(mainLogger, "Modem initialization was not successful");
        return 1;
    } else {
        SPDLOG_LOGGER_INFO(mainLogger, "Modem initialization was successful. Preparing CLI");
    }
    #endif

    std::thread workerThread([&cli, &mainLogger] {
        SPDLOG_LOGGER_INFO(mainLogger, "CLI listener thread started successfully");
        cli.listen();
    });

    modem.worker();

    workerThread.join();
    serial.close();

    return 0;
}
