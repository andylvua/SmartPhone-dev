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
    auto main_logger = spdlog::basic_logger_mt("main", "../logs/log.txt", true);

    spdlog::flush_on(spdlog::level::debug);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] [function %!] [line %#] %v");

    // For debugging purposes only
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
    Screen mainScreen("Main", nullptr);
    mainScreen.addScreenOption("0. Exit");
    mainScreen.addScreenOption("1. Phone");
    mainScreen.addScreenOption("2. SMS");
    mainScreen.addScreenOption("3. USSD Console");
    mainScreen.addScreenOption("4. AT Console");
    mainScreen.addScreenOption("5. Logs");
    Screen incomingCallScreen("Incoming Call", &mainScreen);
    incomingCallScreen.addScreenOption("0. Hang up");
    incomingCallScreen.addScreenOption("1. Answer");
    Screen phoneScreen("Phone", &mainScreen);
    phoneScreen.addScreenOption("0. Back");
    phoneScreen.addScreenOption("1. Call");
    phoneScreen.addScreenOption("2. Contacts");
    Screen callScreen("Call", &phoneScreen);
    callScreen.addScreenOption("0. Return");
    callScreen.addScreenOption("1. Make Call");
    Screen inCallScreen("In Call", &callScreen);
    inCallScreen.addScreenOption("0. Hang up");
    Screen contactsScreen("Contacts", &phoneScreen);
    contactsScreen.addScreenOption("0. Back");
    contactsScreen.addScreenOption("1. Add Contact");
    contactsScreen.addScreenOption("2. Remove Contact");
    contactsScreen.addScreenOption("3. View Contacts");
    Screen smsScreen("SMS", &mainScreen);
    smsScreen.addScreenOption("0. Back");
    smsScreen.addScreenOption("1. Send SMS");
    smsScreen.addScreenOption("2. Messages");
    Screen sendSMSScreen("Send SMS", &smsScreen);
    sendSMSScreen.addScreenOption("0. Back");
    sendSMSScreen.addScreenOption("1. Write SMS");

    CLI cli{modem, &mainScreen};
    cli.addScreen(&mainScreen);
    cli.addScreen(&incomingCallScreen);
    cli.addScreen(&phoneScreen);
    cli.addScreen(&callScreen);
    cli.addScreen(&smsScreen);
    cli.addScreen(&sendSMSScreen);
    cli.addScreen(&inCallScreen);
    cli.addScreen(&contactsScreen);

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

//    QMainWindow window;
//    window.show();
//    return QApplication::exec();
}

