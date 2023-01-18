#include <QApplication>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "Inc/modem/serial.h"
#include "Inc/modem/modem.h"
#include "Inc/cli/cli.h"
#include "Inc/logging.h"
#include <thread>


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    auto main_logger = spdlog::basic_logger_mt("main",
                                               "../logs/log.txt",
                                               true);

    spdlog::flush_on(spdlog::level::debug);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] [function %!] [line %#] %v");

    #ifdef __APPLE__
        const char* portName = "/dev/tty.usbserial-1410";
    #elif __linux__
        const char* portName = "/dev/ttyUSB0";
    #endif

    SerialPort serial(portName,
                      1000,
                      QSerialPort::Baud115200,
                      QSerialPort::Data8,
                      QSerialPort::NoParity,
                      QSerialPort::OneStop,
                      QSerialPort::NoFlowControl);


    if (!serial.open()) {
        qDebug() << "Serial port was not opened" << serial.errorString();
        SPDLOG_LOGGER_ERROR(main_logger, "Serial port was not opened: \"{}\"", serial.errorString().toStdString());
        return 1;
    } else {
        SPDLOG_LOGGER_INFO(main_logger, "Serial port was opened successfully. Starting modem");
    }


    Modem modem{serial};
    CLI cli{modem};

    bool modemReady = modem.initialize();

    if (!modemReady) {
        qDebug() << "Modem initialization failed. Exiting...";
        return 1;
    } else {
        SPDLOG_LOGGER_INFO(main_logger, "Modem initialization was successful. Preparing CLI");
    }

    std::thread workerThread([&cli, &main_logger] {
        SPDLOG_LOGGER_INFO(main_logger, "CLI listener thread started successfully");
        cli.listen();
    });

    modem.worker();

    workerThread.join();
    serial.close();

    return 0;
}

