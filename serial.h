//
// Created by Andrew Yaroshevych on 02.12.2022.
//

#ifndef UNTITLED3_SERIAL_H
#define UNTITLED3_SERIAL_H

#include <QMainWindow>
#include <QApplication>
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QThread>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class SerialPort : public QSerialPort {
    const char *portName;
    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
    QSerialPort::FlowControl flowControl;

public:
    int timeout;

    SerialPort(const char *portName, int timeout, QSerialPort::BaudRate baudRate, QSerialPort::DataBits dataBits,
               QSerialPort::Parity parity, QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControl);

    bool open();
};

#endif //UNTITLED3_SERIAL_H
