#include <QApplication>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "Inc/command.h"
#include "Inc/commands_list.h"
#include "Inc/serial.h"
#include "Inc/modem.h"
#include <thread>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    SerialPort serial("/dev/ttyUSB0",
                      1000,
                      QSerialPort::Baud115200,
                      QSerialPort::Data8,
                      QSerialPort::NoParity,
                      QSerialPort::OneStop,
                      QSerialPort::NoFlowControl);


    if (!serial.open()) {
        qDebug() << "Serial port was not opened" << serial.errorString();
        return 1;
    }

    Modem modem{serial};
    bool modemReady = modem.initialize();
    // run listen thread to be able to receive messages and input commands

    if (modemReady) {
        modem.worker();
    }

    serial.close();

//    QMainWindow window;
//    window.show();
//    return QApplication::exec();
}

