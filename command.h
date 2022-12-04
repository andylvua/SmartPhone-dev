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
private:
    std::string commandText;
    commandType type;

public:
    SerialPort &serial;

    Command(std::string commandText, commandType type, SerialPort &serial);

    std::string getCommandText();

    std::string getType();

    void setCommandText(std::string);

    void setType(commandType commandType);

    virtual void execute() = 0;

    ~Command() = default;
};

class GetCommand : public Command {
public:
    GetCommand(std::string commandText, SerialPort &serial);

    void execute() override;
};

class SetCommand : public Command {
public:
    SetCommand(std::string commandText, SerialPort &serial);

    void execute() override;
};

class Task : public Command {
public:
    Task(std::string commandText, SerialPort &serial);

    void execute() override;
};

class OneTimeCommand : public Command {
public:
    OneTimeCommand(std::string commandText, SerialPort &serial);

    void execute() override;
};

#endif //UNTITLED3_COMMAND_H
