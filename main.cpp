#include <QMainWindow>
#include <QApplication>
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QThread>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "command.h"
void at(QSerialPort& serial);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSerialPort serial;
    serial.setPortName("/dev/ttyUSB0");

    if(serial.open(QIODevice::ReadWrite)){

        //Now the serial port is open try to set configuration
        if(!serial.setBaudRate(QSerialPort::Baud57600))
            qDebug()<<serial.errorString();

        if(!serial.setDataBits(QSerialPort::Data8))
            qDebug()<<serial.errorString();

        if(!serial.setParity(QSerialPort::NoParity))
            qDebug()<<serial.errorString();

        if(!serial.setStopBits(QSerialPort::OneStop))
            qDebug()<<serial.errorString();

        if(!serial.setFlowControl(QSerialPort::NoFlowControl))
            qDebug()<<serial.errorString();

        GetCommand atCommand("AT", serial);
        GetCommand cregCommand("AT+CREG?", serial);
        atCommand.execute();
        cregCommand.execute();
        //I have finish alla operation
        serial.close();
    }else{
        qDebug()<<"Serial ttyUSB0 not opened. Error: "<<serial.errorString();
    }

    QMainWindow w;
    w.show();

    return a.exec();
}

