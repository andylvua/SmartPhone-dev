//
// Created by Andrew Yaroshevych on 02.12.2022.
//
#include <QtSerialPort/QSerialPort>
#include "serial.h"

SerialPort::SerialPort(const char *portName, int timeout, QSerialPort::BaudRate baudRate, QSerialPort::DataBits dataBits,
                       QSerialPort::Parity parity, QSerialPort::StopBits stopBits,
                       QSerialPort::FlowControl flowControl) {
    this->portName = portName;
    this->timeout = timeout;
    this->baudRate = baudRate;
    this->dataBits = dataBits;
    this->parity = parity;
    this->stopBits = stopBits;
    this->flowControl = flowControl;
}

bool SerialPort::open() {
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
