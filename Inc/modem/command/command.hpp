//
// Created by paul on 12/1/22.
//

#ifndef PHONE_COMMAND_HPP
#define PHONE_COMMAND_HPP

#include <string>
#include <QSerialPort>
#include <vector>
#include "modem/serial.hpp"

using commRes_t = enum class commRes {
    CR_OK,
    CR_ERROR,
    CR_TIMEOUT
};

class Command {
public:
    QByteArray commandData;
    SerialPort &serial;

    Command(const QString &commandText, SerialPort &serial);

    static QString uartResponseParser(const QByteArray &response, const QString &commandText);

    static QString uartEchoParser(const QByteArray &response);

    ~Command() = default;
};

class GetCommand : public Command {
public:
    using Command::Command;

    QString execute(bool enableInterruptDataRead = true, bool parseResponse = true);
};

class SetCommand : public Command {
public:
    using Command::Command;

    commRes_t execute(bool enableInterruptDataRead = true);
};

class Task : public Command {
public:
    using Command::Command;

    commRes_t execute(bool parseResponse = true);
};

#endif //PHONE_COMMAND_HPP
