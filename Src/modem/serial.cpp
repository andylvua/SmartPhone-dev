//
// Created by Andrew Yaroshevych on 02.12.2022.
//
#include "modem/serial.hpp"

SerialPort::SerialPort(const char *portName,
                       int timeout,
                       QSerialPort::BaudRate baudRate,
                       QSerialPort::DataBits dataBits,
                       QSerialPort::Parity parity,
                       QSerialPort::StopBits stopBits,
                       QSerialPort::FlowControl flowControl
                       ) : portName(portName), baudRate(baudRate),
                           dataBits(dataBits), parity(parity),
                           stopBits(stopBits), flowControl(flowControl),
                           timeout(timeout) {}

bool SerialPort::openSerialPort() {
    setPortName(portName);
    if (QSerialPort::open(QIODevice::ReadWrite)) {
        if (!setBaudRate(baudRate))
            qDebug() << errorString();

        if (!setDataBits(dataBits))
            qDebug() << errorString();

        if (!setParity(parity))
            qDebug() << errorString();

        if (!setStopBits(stopBits))
            qDebug() << errorString();

        if (!setFlowControl(flowControl))
            qDebug() << errorString();

        return true;
    } else {
        return false;
    }
}
