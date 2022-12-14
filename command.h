//
// Created by paul on 12/1/22.
//

#ifndef UNTITLED3_COMMAND_H
#define UNTITLED3_COMMAND_H

#include <string>
#include <QSerialPort>
#include <vector>
#include "serial.h"

enum commandType {
    getCommand = 0, setCommand = 1, task = 2, oneTimeCommand = 3
};

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

    ~Command() = default;
};

class GetCommand : public Command {
public:
    GetCommand(std::string commandText, SerialPort &serial);

    QString execute();
    friend QString uartResponseParser(const QByteArray& response);
};

class SetCommand : public Command {
public:
    SetCommand(std::string commandText, SerialPort &serial);

    void execute();
};

class Task : public Command {
public:
    Task(std::string commandText, SerialPort &serial);

    void execute();
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
