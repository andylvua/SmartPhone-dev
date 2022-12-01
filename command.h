//
// Created by paul on 12/1/22.
//

#ifndef UNTITLED3_COMMAND_H
#define UNTITLED3_COMMAND_H

#include <string>
#include <QSerialPort>
#pragma once
enum commandType{
    getCommand = 0, setCommand = 1, task = 2, oneTimeCommand = 3
};
const char* commandTypeStr[] = {"getCommand",  "setCommand", "task","oneTimeCommand"};
class Command{
private:
    std::string commandText;
    commandType type;
    QSerialPort& serial;
public:
    Command(std::string commandText, commandType type, QSerialPort& serial);

    const std::string getCommandText();

    const std::string getType();

    void setCommandText(std::string);

    void setType(commandType type);
};
class GetCommand : public Command{
    GetCommand(std::string commandText, QSerialPort& serial);

    std::string execute();
};
class SetCommand : public Command{
    SetCommand(std::string commandText, QSerialPort& serial);

    void execute();
};
class Task : public Command{
    Task(std::string commandText, QSerialPort& serial);

    void execute();
};
class OneTimeCommand : public Command{
    OneTimeCommand(std::string commandText, QSerialPort& serial);

    void execute();
};

#endif //UNTITLED3_COMMAND_H
