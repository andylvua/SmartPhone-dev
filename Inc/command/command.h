//
// Created by paul on 12/1/22.
//

#ifndef UNTITLED3_COMMAND_H
#define UNTITLED3_COMMAND_H

#include <string>
#include <QSerialPort>
#include <vector>
#include "../modem/serial.h"

using commRes_t = enum class commRes {
    CR_OK,
    CR_ERROR,
    CR_TIMEOUT
};

class Command {
public:
    std::string commandText;
    SerialPort &serial;

    Command(std::string commandText, SerialPort &serial);

    std::string getCommandText() const;

    static QString uartResponseParser(const QByteArray &response);

    static QString uartEchoParser(const QByteArray &response);

    ~Command() = default;
};

class GetCommand : public Command {
public:
    GetCommand(const std::string &commandText, SerialPort &serial);

    QString execute(bool enableInterruptDataRead = true, bool parseResponse = true);
};

class SetCommand : public Command {
public:
    SetCommand(const std::string &commandText, SerialPort &serial);

    commRes_t execute(bool enableInterruptDataRead = true);
};

class Task : public Command {
public:
    Task(const std::string &commandText, SerialPort &serial);

    commRes_t execute(bool parseResponse = true);
};

#endif //UNTITLED3_COMMAND_H
