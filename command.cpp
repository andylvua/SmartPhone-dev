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

Command::Command(std::string commandText, commandType type, SerialPort &serial) : type(type), serial(serial) {
    this->commandText = std::move(commandText);
}

std::string Command::getCommandText() const{
    return commandText;
}

std::string Command::getType() const {
    return commandTypeStr[type];
}

void Command::setCommandText(std::string command) {
    this->commandText = std::move(command);
}

void Command::setType(commandType commandType) {
    this->type = commandType;
}
// returns actual response from uart dropping echo of command
QString uartResponseParser(const QByteArray& response) {
    auto responseString = QString(response);
    QStringList parsedResponse;
    parsedResponse = responseString.split("\r\n");
    return QString{parsedResponse[1]};
}
GetCommand::GetCommand(std::string commandText, SerialPort &serial) :
    Command(std::move(commandText), commandType::getCommand,serial) {}


QString GetCommand::execute() {
    qDebug() << ("Request: "+getCommandText()).c_str();
    serial.write((getCommandText() + "\r\n").c_str());
    QByteArray data;
    if (serial.waitForReadyRead(serial.timeout)) {
        data = serial.readAll();
        while (serial.waitForReadyRead(serial.timeout))
            data += serial.readAll();
    } else {
        qDebug() << "Timeout";
    }
    // not tested part
    QString response = uartResponseParser(data);
    if (response.isValidUtf16() && !response.isNull() && !response.isEmpty()){
        qDebug() << "Response: "+ response;
    }
    else{
        qDebug() << "Error occurred";
    }
    return response;
}


SetCommand::SetCommand(std::string commandText, SerialPort &serial) :
    Command(std::move(commandText), commandType::setCommand,serial) {}

void SetCommand::execute() {
    qDebug() << ("Request: "+getCommandText()).c_str();
    serial.write((getCommandText() + "\r\n").c_str());
    QByteArray data;
    if (serial.waitForReadyRead(serial.timeout)) {
        data = serial.readAll();
        while (serial.waitForReadyRead(serial.timeout))
            data += serial.readAll();
    } else {
        qDebug() << "Timeout";
    }
    // not tested part
    QString response = uartResponseParser(data);
    // I don't know if there is only OK answer for such prompts
    if (response.isValidUtf16() && !response.isNull() && !response.isEmpty() && response == "OK"){
        qDebug() << "Response: "+ response;
    }
    else {
        qDebug() << "Error occurred";
    }
}

Task::Task(std::string commandText, SerialPort &serial) :
    Command(std::move(commandText), commandType::task, serial) {};
// should be maintained as async process
void Task::execute() {
    // To do
    qDebug() << ("Request: "+getCommandText()).c_str();
    serial.write((getCommandText() + "\r\n").c_str());
    QByteArray data;
    while (true){
        if (serial.waitForReadyRead(10000)) {
            data = serial.readAll();
            while (serial.waitForReadyRead(serial.timeout))
                data += serial.readAll();
            QString response = uartResponseParser(data);
            if (response.contains("Error")){
                qDebug() << "Timeout";
                break;
            }
        } else {
            qDebug() << "Timeout";
            break;
        }
    }
}
// redundant
//
//OneTimeCommand::OneTimeCommand(std::string commandText, SerialPort &serial) :
//    Command(std::move(commandText),commandType::oneTimeCommand,serial) {};
//
//void OneTimeCommand::execute() {
//}