//
// Created by paul on 12/1/22.
//

#ifndef UNTITLED3_COMMAND_H
#define UNTITLED3_COMMAND_H

#include <string>
#include <QSerialPort>
#include <vector>
#include "../modem/serial.h"

enum commandType {
    getCommand = 0, setCommand = 1, task = 2, oneTimeCommand = 3
};

typedef enum {
    CR_OK,
    CR_ERROR,
    CR_TIMEOUT
} commRes_t;

const std::vector<std::string> commandTypeStr = {"getCommand", "setCommand", "task", "oneTimeCommand"};

class Command {
public:
    std::string commandText;
    commandType type;
    SerialPort &serial;

    Command(std::string commandText, commandType type, SerialPort &serial);

    std::string getCommandText() const;

    std::string getType() const;

    void setCommandText(std::string);

    void setType(commandType commandType);

    static QString uartResponseParser(const QByteArray &response);

    static QString uartEchoParser(const QByteArray &response);

    ~Command() = default;
};

class GetCommand : public Command {
public:
    GetCommand(std::string commandText, SerialPort &serial);

    QString execute(bool enableInterruptDataRead = true, bool parseResponse = true);
};

class SetCommand : public Command {
public:
    SetCommand(std::string commandText, SerialPort &serial);

    commRes_t execute(bool enableInterruptDataRead = true);
};

class Task : public Command {
public:
    Task(std::string commandText, SerialPort &serial);

    commRes_t execute(bool parseResponse = true);
};
//redundant
//
//class OneTimeCommand : public Command {
//public:
//    OneTimeCommand(std::string commandText, SerialPort &serial);
//
//    void execute() override;
//};


#endif //UNTITLED3_COMMAND_H
