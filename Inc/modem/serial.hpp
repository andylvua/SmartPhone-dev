//
// Created by Andrew Yaroshevych on 02.12.2022.
//

#ifndef PHONE_SERIAL_HPP
#define PHONE_SERIAL_HPP

#include <QMainWindow>
#include <QApplication>
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QThread>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class SerialPort : public QSerialPort {
public:
    const char *portName;
    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
    QSerialPort::FlowControl flowControl;
    int timeout;
    bool interruptDataRead = false;
    QByteArray buffer;

    SerialPort(const char *portName, int timeout, QSerialPort::BaudRate baudRate, QSerialPort::DataBits dataBits,
               QSerialPort::Parity parity, QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControl);

    bool openSerialPort();
};

#endif //PHONE_SERIAL_HPP
