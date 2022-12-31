//#include <QMainWindow>
//#include <QDebug>
//#include <QTextStream>
//#include <QFile>
//#include <QThread>
#include <QApplication>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "Inc/command.h"
#include "Inc/commands_list.h"
#include "Inc/serial.h"
#include "Inc/modem.h"
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    SerialPort serial("/dev/tty.usbserial-1130",
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
    modem.call("+380980699038");

    GetCommand atCommand(AT, serial);
    GetCommand cregCommand(AT_CREG"?", serial);
    atCommand.execute();
    cregCommand.execute();

    serial.close();



//    QMainWindow window;
//    window.show();
//    return QApplication::exec();
    return 0;
}

