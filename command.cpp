//
// Created by paul on 12/1/22.
//
#include "command.h"

Command::Command(std::string commandText, commandType type, QSerialPort& serial) : serial(serial){
    this->commandText = commandText;
    this->type = type;
}

const std::string Command::getCommandText(){
    return commandText;
}

const std::string Command::getType() {
    return commandTypeStr[type];
}
void Command::setCommandText(std::string command) {
    commandText = command;
}
void Command::setType(commandType type) {
    this->type = type;
}
GetCommand::GetCommand(std::string commandText, QSerialPort& serial) : Command(commandText, commandType::getCommand, serial){
}
std::string GetCommand::execute() {
}
SetCommand::SetCommand(std::string commandText, QSerialPort& serial) : Command(commandText, commandType::setCommand, serial){};
void SetCommand::execute() {
}
Task::Task(std::string commandText, QSerialPort& serial) : Command(commandText, commandType::task, serial){};
void Task::execute() {
}
OneTimeCommand::OneTimeCommand(std::string commandText, QSerialPort &serial) : Command(commandText, commandType::oneTimeCommand, serial){};
void OneTimeCommand::execute() {
}