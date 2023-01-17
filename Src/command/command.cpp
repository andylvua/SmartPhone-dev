//
// Created by paul on 12/1/22.
//

#include "../../Inc/command/command.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QThread>
#include <QDebug>
#include "../../Inc/modem/serial.h"
#include <iostream>
#include "../../Inc/logging.h"

auto command_logger = spdlog::basic_logger_mt("command", "../logs/log.txt", true);

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
    SPDLOG_LOGGER_INFO(command_logger, "uartResponseParser: {}", response.toStdString());
    auto responseString = QString(response);
    if (responseString.isEmpty()) {
        return responseString;
    }
    QStringList parsedResponse;
    parsedResponse = responseString.split("\r\n");

    if (parsedResponse.size() > 2) {
        return QString{parsedResponse[2]};
        SPDLOG_LOGGER_INFO(command_logger, "uartResponseParser [parsed]: {}", parsedResponse[2].toStdString());
    } else {
        qDebug() << "WARNING: Response from UART was not parsed correctly";
        SPDLOG_LOGGER_WARN(command_logger, "Response from UART was not parsed correctly, returning empty string");
        return QString{};
    }
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


QString GetCommand::execute(bool enableInterruptDataRead, bool parseResponse) {
    SPDLOG_LOGGER_INFO(command_logger, "Executing GET command: {}", commandText);
    auto request = QString::fromStdString(commandText);
    qDebug() << ("Request: " + request);

    if (enableInterruptDataRead) {
        SPDLOG_LOGGER_INFO(command_logger, "Interrupt data read enabled");
        serial.interruptDataRead = true;
    }

    serial.write((getCommandText() + "\r\n").c_str());

    while (serial.interruptDataRead) {
        QThread::msleep(100);
    }

    QByteArray data;

    if (enableInterruptDataRead) {
        data = serial.buffer;
        SPDLOG_LOGGER_INFO(command_logger, "Interrupt data read disabled. Data received: {}", data.toStdString());
    } else {
        if (serial.waitForReadyRead(serial.timeout)) {
            data = serial.readAll();
            while (serial.waitForReadyRead(serial.timeout))
                data += serial.readAll();
        } else {
            qDebug() << "Timeout";
            SPDLOG_LOGGER_WARN(command_logger, "Timeout");
        }
    }

    if (data.isEmpty()) {
        qDebug() << "WARNING: No data received during execution of GetCommand";
        SPDLOG_LOGGER_WARN(command_logger, "No data received during execution of GetCommand");
        return QString{};
    }

    if (!parseResponse) {
        return QString{data};
    }

    QString response = uartResponseParser(data);

    if (response.isValidUtf16() && !response.isNull()) {
        if (request != uartEchoParser(data)) {
            qDebug() << "WARNING: Echo of command does not match request";
            SPDLOG_LOGGER_WARN(command_logger, "Echo of command does not match request");
        }

        qDebug() << ("Response: " + response);
        SPDLOG_LOGGER_INFO(command_logger, "Response: {}", response.toStdString());
    } else {
        qDebug() << "Error: invalid response";
        SPDLOG_LOGGER_WARN(command_logger, "Invalid response");
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
        Command(std::move(commandText), commandType::task, serial) {}

commRes_t Task::execute(bool parseResponse) {
    SPDLOG_LOGGER_INFO(command_logger, "Executing task: {}", commandText);
    auto request = QString::fromStdString(commandText);
    qDebug() << ("Request: " + request);

    serial.interruptDataRead = true;
    SPDLOG_LOGGER_INFO(command_logger, "Interrupt data read enabled");

    serial.write((getCommandText() + "\r\n").c_str());

    while (serial.interruptDataRead) {
        QThread::msleep(100);
    }

    QByteArray data = serial.buffer;
    SPDLOG_LOGGER_INFO(command_logger, "Interrupt data read disabled. Data received: {}", data.toStdString());

    if (data.isEmpty()) {
        qDebug() << "Timeout";
        SPDLOG_LOGGER_WARN(command_logger, "Timeout");
        return CR_TIMEOUT;
    }

    if (!parseResponse) {
        return CR_OK;
    }

    QString response = uartResponseParser(data);
    qDebug() << ("Unparsed response: " + QString(data));
    SPDLOG_LOGGER_INFO(command_logger, "Unparsed response: {}", QString(data).toStdString());

    if (response.isValidUtf16() && !response.isNull()) {
        if (response.contains("ERROR")) {
            qDebug() << "ERROR: Response contains ERROR";
            SPDLOG_LOGGER_WARN(command_logger, "Response contains ERROR");
            return commRes_t::CR_ERROR;
        }

        qDebug() << ("Response: " + response);
        SPDLOG_LOGGER_INFO(command_logger, "Response: {}", response.toStdString());
        return commRes_t::CR_OK;
    } else {
        qDebug() << "Error: invalid response";
        SPDLOG_LOGGER_WARN(command_logger, "Invalid response");
        return commRes_t::CR_ERROR;
    }
}
