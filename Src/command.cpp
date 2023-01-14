//
// Created by paul on 12/1/22.
//
#include "../Inc/command.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTextStream>
#include <QThread>
#include <QDebug>
#include "../Inc/serial.h"
#include <iostream>

Command::Command(std::string commandText, commandType type, SerialPort &serial) : type(type), serial(serial) {
    this->commandText = std::move(commandText);
}

std::string Command::getCommandText() const {
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

// Returns actual response from uart, dropping echo of command
QString Command::uartResponseParser(const QByteArray &response) {
    auto responseString = QString(response);
    if (responseString.isEmpty()) {
        return responseString;
    }
    QStringList parsedResponse;
    parsedResponse = responseString.split("\r\n");
    return QString{parsedResponse[2]};
}

// Returns echo of command, dropping actual response from uart
QString Command::uartEchoParser(const QByteArray &response) {
    auto responseString = QString(response);
    if (responseString.isEmpty()) {
        return responseString;
    }
    QStringList parsedResponse;
    parsedResponse = responseString.split("\r\n");
    return QString{parsedResponse[0]};
}

GetCommand::GetCommand(std::string commandText, SerialPort &serial) :
        Command(std::move(commandText), commandType::getCommand, serial) {}


QString GetCommand::execute() {
    auto request = QString::fromStdString(commandText);
    qDebug() << ("Request: " + request);

    serial.write((getCommandText() + "\r\n").c_str());

    QByteArray data;
    if (serial.waitForReadyRead(serial.timeout)) {
        data = serial.readAll();
        while (serial.waitForReadyRead(serial.timeout))
            data += serial.readAll();
    } else {
        qDebug() << "Timeout";
    }

    QString response = uartResponseParser(data);

    if (response.isValidUtf16() && !response.isNull() && !response.isEmpty()) {
        if (request != uartEchoParser(data)) {
            qDebug() << "Echo is not equal to request";
        }

        qDebug() << ("Response: " + response);
    } else {
        qDebug() << "Error: invalid response";
    }
    return response;
}


SetCommand::SetCommand(std::string commandText, SerialPort &serial) :
        Command(std::move(commandText), commandType::setCommand, serial) {}

commRes_t SetCommand::execute() {
    auto request = QString::fromStdString(commandText);
    qDebug() << ("Request: " + getCommandText()).c_str();

    serial.write((getCommandText() + "\r\n").c_str());

    QByteArray data;
    if (serial.waitForReadyRead(serial.timeout)) {
        data = serial.readAll();
        while (serial.waitForReadyRead(serial.timeout))
            data += serial.readAll();
    } else {
        qDebug() << "Timeout";
        return CR_TIMEOUT;
    }

    QString response = uartResponseParser(data);

    if (response.isValidUtf16() && !response.isNull() && !response.isEmpty() && response == "OK") {
        if (request != uartEchoParser(data)) {
            qDebug() << "Echo is not equal to request";
            return CR_ERROR;
        }

        qDebug() << ("Response: " + response);
    } else {
        qDebug() << "Error: invalid response";
        return CR_ERROR;
    }

    return CR_OK;
}

Task::Task(std::string commandText, SerialPort &serial) :
        Command(std::move(commandText), commandType::task, serial) {};

commRes_t Task::execute() {
    auto request = QString::fromStdString(commandText);
    qDebug() << ("Request: " + request);

    serial.write((getCommandText() + "\r\n").c_str());

    QByteArray data;
    if (serial.waitForReadyRead(serial.timeout)) {
        data = serial.readAll();
        while (serial.waitForReadyRead(serial.timeout))
            data += serial.readAll();
    } else {
        qDebug() << "Timeout";
        return commRes_t::CR_TIMEOUT;
    }

    QString response = uartResponseParser(data);

    if (response.isValidUtf16() && !response.isNull() && !response.isEmpty()) {
        if (request != uartEchoParser(data)) {
            qDebug() << "Echo is not equal to request. Actual echo: " << uartEchoParser(data)
                     << " Request: " << request;
            return commRes_t::CR_ERROR;
        }

        if (response.indexOf("OK") == -1) {
            qDebug() << "Error: invalid response";
            return commRes_t::CR_ERROR;
        }

        qDebug() << ("Response: " + response);
        return commRes_t::CR_OK;
    } else {
        qDebug() << "Error: invalid response";
        return commRes_t::CR_ERROR;
    }
}


// redundant
//
//OneTimeCommand::OneTimeCommand(std::string commandText, SerialPort &serial) :
//    Command(std::move(commandText),commandType::oneTimeCommand,serial) {};
//
//void OneTimeCommand::execute() {
//}