//
// Created by paul on 12/1/22.
//

#ifndef UNTITLED3_COMMAND_H
#define UNTITLED3_COMMAND_H

#include <string>
#include <QSerialPort>
#include <vector>

enum commandType{
    getCommand = 0, setCommand = 1, task = 2, oneTimeCommand = 3
};

const std::vector<std::string> commandTypeStr = {"getCommand",  "setCommand", "task","oneTimeCommand"};

class Command{
private:
    std::string commandText;
    commandType type;

public:
    QSerialPort& serial;

    Command(std::string commandText, commandType type, QSerialPort& serial);

    const std::string getCommandText();

    const std::string getType();

    void setCommandText(std::string);

    void setType(commandType type);
};
class GetCommand : public Command{
public:
    GetCommand(std::string commandText, QSerialPort& serial);

    void execute();
};
class SetCommand : public Command{
public:
    SetCommand(std::string commandText, QSerialPort& serial);

    void execute();
};
class Task : public Command{
public:
    Task(std::string commandText, QSerialPort& serial);

    void execute();
};
class OneTimeCommand : public Command{
public:
    OneTimeCommand(std::string commandText, QSerialPort& serial);

    void execute();
};

#endif //UNTITLED3_COMMAND_H
