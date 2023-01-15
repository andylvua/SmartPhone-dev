#include <QApplication>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "Inc/serial.h"
#include "Inc/modem.h"
#include "Inc/cli.h"
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
    CLI cli{modem};

    bool modemReady = modem.initialize();

    if (!modemReady) {
        qDebug() << "Modem initialization failed. Exiting...";
        return 1;
    }

    std::thread workerThread([&modem] {
        modem.worker();
    });

    cli.listen();

    workerThread.join();
    serial.close();

//    QMainWindow window;
//    window.show();
//    return QApplication::exec();
}

