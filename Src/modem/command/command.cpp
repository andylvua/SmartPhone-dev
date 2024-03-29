//
// Created by paul on 12/1/22.
//

#include "logging.hpp"
#include "modem/command/command.hpp"
#include "modem/utils/cache_manager.hpp"
#include <QSerialPortInfo>
#include <QThread>

const auto commandLogger = spdlog::basic_logger_mt("command",
                                                   LOGS_FILEPATH, true);

Command::Command(const QString &commandText, SerialPort &serial) : commandData(commandText.toUtf8()), serial(serial) {}

// Returns actual response from uart, dropping echo of command
QString Command::uartResponseParser(const QByteArray &response, const QString &commandText) {
    SPDLOG_LOGGER_INFO(commandLogger, "uartResponseParser: {}", response.toStdString());
    auto responseString = QString(response);
    if (responseString.isEmpty()) {
        return responseString;
    }

    QStringList parsedResponseList = responseString.split("\r\n");

    parsedResponseList.removeAll("");
    parsedResponseList.removeOne(commandText);

    QString parsedResponse = parsedResponseList.join("\n");

    SPDLOG_LOGGER_INFO(commandLogger, "uartResponseParser [parsed]: {}", parsedResponse.toStdString());
    return parsedResponse;
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

QString GetCommand::execute(bool enableInterruptDataRead, bool parseResponse) {
    SPDLOG_LOGGER_INFO(commandLogger, "Executing GET command: {}", commandData.toStdString());

    if (enableInterruptDataRead) {
        SPDLOG_LOGGER_INFO(commandLogger, "Interrupt data read enabled");
        serial.interruptDataRead = true;
    }

    serial.write(commandData + "\r\n");

    while (serial.interruptDataRead) {
        QThread::msleep(100);
    }

    QByteArray data;

    if (enableInterruptDataRead) {
        data = serial.buffer;
        SPDLOG_LOGGER_INFO(commandLogger, "Interrupt data read disabled. Data received: {}", data.toStdString());
    } else {
        if (serial.waitForReadyRead(serial.timeout)) {
            data = serial.readAll();
            while (serial.waitForReadyRead(serial.timeout))
                data += serial.readAll();
        } else {
            qDebug() << "Timeout";
            SPDLOG_LOGGER_WARN(commandLogger, "Timeout");
        }
    }

    if (data.isEmpty()) {
        qDebug() << "WARNING: No data received during execution of GetCommand";
        SPDLOG_LOGGER_WARN(commandLogger, "No data received during execution of GetCommand");
        return QString{};
    }

    if (!parseResponse) {
        return QString{data};
    }

    QString response = uartResponseParser(data, commandData);

    if (response.isValidUtf16() && !response.isNull()) {
        if (commandData != uartEchoParser(data)) {
            qDebug() << "WARNING: Echo of command does not match request";
            SPDLOG_LOGGER_WARN(commandLogger, "Echo of command does not match request");
        }

        SPDLOG_LOGGER_INFO(commandLogger, "Response: {}", response.toStdString());
    } else {
        SPDLOG_LOGGER_WARN(commandLogger, "Invalid response");
    }

    return response;
}

commRes_t SetCommand::execute([[maybe_unused]] bool enableInterruptDataRead) {
    SPDLOG_LOGGER_INFO(commandLogger, "Executing SET command: {}", commandData.toStdString());

    if (enableInterruptDataRead) {
        SPDLOG_LOGGER_INFO(commandLogger, "Interrupt data read enabled");
        serial.interruptDataRead = true;
    }

    serial.write(commandData + "\r\n");

    while (serial.interruptDataRead) {
        QThread::msleep(100);
    }

    QByteArray data;

    if (enableInterruptDataRead) {
        data = serial.buffer;
        SPDLOG_LOGGER_INFO(commandLogger, "Interrupt data read disabled. Data received: {}", data.toStdString());
    } else {
        if (serial.waitForReadyRead(serial.timeout)) {
            data = serial.readAll();
            while (serial.waitForReadyRead(serial.timeout))
                data += serial.readAll();
        } else {
            qDebug() << "Timeout";
            return commRes::CR_TIMEOUT;
        }
    }

    if (data.isEmpty()) {
        qDebug() << "WARNING: No data received during execution of SetCommand";
        SPDLOG_LOGGER_WARN(commandLogger, "No data received during execution of GetCommand");
    }

    if (data.contains("OK")) {
        if (commandData != uartEchoParser(data)) {
            SPDLOG_LOGGER_WARN(commandLogger, "Echo is not equal to request");
        }
    } else {
        SPDLOG_LOGGER_WARN(commandLogger, "Invalid response");
        return commRes::CR_ERROR;
    }

    return commRes::CR_OK;
}

commRes_t Task::execute(bool parseResponse) {
    SPDLOG_LOGGER_INFO(commandLogger, "Executing task: {}", commandData.toStdString());

    serial.interruptDataRead = true;
    SPDLOG_LOGGER_INFO(commandLogger, "Interrupt data read enabled");

    serial.write(commandData + "\r\n");

    while (serial.interruptDataRead) {
        QThread::msleep(100);
    }

    QByteArray data = serial.buffer;
    SPDLOG_LOGGER_INFO(commandLogger, "Interrupt data read disabled. Data received: {}", data.toStdString());

    if (data.isEmpty()) {
        qDebug() << "Timeout";
        SPDLOG_LOGGER_WARN(commandLogger, "Timeout");
        return commRes::CR_TIMEOUT;
    }

    if (!parseResponse) {
        return commRes::CR_OK;
    }

    QString response = uartResponseParser(data, commandData);
    SPDLOG_LOGGER_INFO(commandLogger, "Unparsed response: {}", QString(data).toStdString());

    if (response.isValidUtf16() && !response.isNull()) {
        if (response.contains("ERROR")) {
            qDebug() << "ERROR: Response contains ERROR";
            SPDLOG_LOGGER_WARN(commandLogger, "Response contains ERROR");
            return commRes_t::CR_ERROR;
        }

        SPDLOG_LOGGER_INFO(commandLogger, "Response: {}", response.toStdString());
        return commRes_t::CR_OK;
    } else {
        SPDLOG_LOGGER_WARN(commandLogger, "Invalid response");
        return commRes_t::CR_ERROR;
    }
}
