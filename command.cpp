//
// Created by paul on 12/1/22.
//
#include "command.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTextStream>
#include <QThread>
#include <QDebug>
#include "serial.h"

Command::Command(std::string commandText, commandType type, SerialPort &serial) : serial(serial) {
    this->commandText = std::move(commandText);
    this->type = type;
}

std::string Command::getCommandText() {
    return commandText;
}

std::string Command::getType() {
    return commandTypeStr[type];
}

void Command::setCommandText(std::string command) {
    this->commandText = std::move(command);
}

void Command::setType(commandType commandType) {
    this->type = commandType;
}

GetCommand::GetCommand(std::string commandText, SerialPort &serial) :
    Command(std::move(commandText), commandType::getCommand,serial) {}


void GetCommand::execute() {
//    serial.write((getCommandText() + "\r\n").c_str());
//
//    if (serial.waitForReadyRead(2000)) {
//        //Data was returned
//        qDebug() << ("Request: "+getCommandText()).c_str();
//    } else {
//        //No data
//        qDebug() << "Time out";
//    }
//
//    if (serial.waitForReadyRead(2000)) {
//        //Data was returned
//        qDebug() << "Response: "<<serial.readAll();
////        qDebug()<<"Response2: "<<serial.readAll();
//    } else {
//        //No data
//        qDebug() << "Time out";
//    }

    qDebug() << ("Request: "+getCommandText()).c_str();
    serial.write((getCommandText() + "\r\n").c_str());

    if (serial.waitForReadyRead(serial.timeout)) {
        QByteArray data = serial.readAll();
        while (serial.waitForReadyRead(serial.timeout))
            data += serial.readAll();
        qDebug() << data;
    } else {
        qDebug() << "Timeout";
    }
}

SetCommand::SetCommand(std::string commandText, SerialPort &serial) :
    Command(std::move(commandText), commandType::setCommand,serial) {}

void SetCommand::execute() {
    serial.write((getCommandText() + "\r\n").c_str());
    //the serial must remain opened
    QThread::msleep(2000);
    if (serial.waitForReadyRead(2000)) {
        //Data was returned
        qDebug() << ("Request: "+getCommandText()).c_str() ;
    } else {
        //No data
        qDebug() << "Time out";
    }
    QThread::msleep(2000);
    if (serial.waitForReadyRead(2000)) {
        //Data was returned
        qDebug() << "Response: "<<serial.readAll();
//        qDebug()<<"Response2: "<<serial.readAll();
    } else {
        //No data
        qDebug() << "Time out";
    }
}

Task::Task(std::string commandText, SerialPort &serial) :
    Command(std::move(commandText), commandType::task, serial) {};

void Task::execute() {
}

OneTimeCommand::OneTimeCommand(std::string commandText, SerialPort &serial) :
    Command(std::move(commandText),commandType::oneTimeCommand,serial) {};

void OneTimeCommand::execute() {
}