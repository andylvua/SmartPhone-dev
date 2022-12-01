#include <QMainWindow>
#include <QApplication>
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QThread>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
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

        at(serial);
        //I have finish alla operation
        serial.close();
    }else{
        qDebug()<<"Serial ttyUSB0 not opened. Error: "<<serial.errorString();
    }

    QMainWindow w;
    w.show();

    return a.exec();
}

void at(QSerialPort& serial){
    //If any error was returned the serial il correctly configured
    serial.write("AT\r\n");
    //the serial must remain opened
    QThread::msleep(2000);
    if(serial.waitForReadyRead(2000)){
        //Data was returned
        qDebug()<<"Request: "<<serial.readLine();
    }else{
        //No data
        qDebug()<<"Time out";
    }
    QThread::msleep(2000);
    if(serial.waitForReadyRead(2000)){
        //Data was returned
        qDebug()<<"Response: "<<serial.readAll();
//        qDebug()<<"Response2: "<<serial.readAll();
    }else{
        //No data
        qDebug()<<"Time out";
    }
}
