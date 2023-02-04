//
// Created by Andrew Yaroshevych on 21.12.2022.
//

#include "logging.hpp"
#include "modem/modem.hpp"
#include "modem/utils/cache_manager.hpp"
#include "modem/command/command.hpp"
#include "modem/command/commands_list.hpp"
#include "cli/utils/io/ncurses_io.hpp"
#include "cli/definitions/colors.hpp"
#include <string>
#include <utility>

const auto modemLogger = spdlog::basic_logger_mt("modem", "../logs/log.txt", true);

Modem::Modem(SerialPort &serial) : serial(serial) {}

bool Modem::enableEcho() {
    SPDLOG_LOGGER_INFO(modemLogger, "Enabling echo");
    auto enableEchoCommand = SetCommand(ATE1, serial);

    auto response = enableEchoCommand.execute(false);

    if (response == commRes::CR_OK) {
        SPDLOG_LOGGER_INFO(modemLogger, "Echo enabled successfully");
        return true;
    } else {
        SPDLOG_LOGGER_ERROR(modemLogger, "Echo enabled failed");
        return false;
    }
}

void Modem::setCharacterSet(const std::string &characterSet) {
    SPDLOG_LOGGER_INFO(modemLogger, "Setting character set to {}", characterSet);
    auto setCharacterSetCommand = SetCommand(AT_CSCS"=" + characterSet, serial);
    setCharacterSetCommand.execute();
}

void Modem::enableATConsoleMode() {
    SPDLOG_LOGGER_INFO(modemLogger, "Console mode enabled");
    serial.write(ATE0"\r\n");
    QThread::msleep(serial.timeout);
    consoleMode.enabled = true;
    consoleMode.consoleType = consoleType::CM_AT;
}

void Modem::disableATConsoleMode() {
    SPDLOG_LOGGER_INFO(modemLogger, "Console mode disabled");
    consoleMode.enabled = false;
    serial.write(ATE1"\r\n");
}

void Modem::enableUSSDConsoleMode() {
    SPDLOG_LOGGER_INFO(modemLogger, "USSD console mode enabled");
    setCharacterSet("HEX");
    serial.write(ATE0"\r\n");
    QThread::msleep(serial.timeout);
    consoleMode.enabled = true;
    consoleMode.consoleType = consoleType::CM_USSD;
}

void Modem::disableUSSDConsoleMode() {
    SPDLOG_LOGGER_INFO(modemLogger, "USSD console mode disabled");
    consoleMode.enabled = false;
    serial.write(ATE1"\r\n");
}

bool Modem::enableHTTPConsoleMode() {
    SPDLOG_LOGGER_INFO(modemLogger, "HTTP console mode enabled");

    serial.write(ATE0"\r\n");
    QThread::msleep(serial.timeout);

    auto setAPNCommand = SetCommand(R"(AT+CGDCONT=1,"IP","internet")", serial);
    auto response = setAPNCommand.execute();

    if (response != commRes::CR_OK) {
        SPDLOG_LOGGER_ERROR(modemLogger, "Failed to set APN");
        std::cout << RED_COLOR << "\nFailed to set APN" << RESET << std::endl;
        QThread::msleep(3000);
        return false;
    }

    auto activateConnectionCommand = SetCommand("AT+CGACT=1,1", serial);
    response = activateConnectionCommand.execute();
    if (response != commRes::CR_OK) {
        SPDLOG_LOGGER_ERROR(modemLogger, "Failed to activate connection");
        std::cout << RED_COLOR << "\nFailed to activate connection" << RESET << std::endl;
        QThread::msleep(3000);
        return false;
    }

    consoleMode.enabled = true;
    consoleMode.consoleType = consoleType::CM_HTTP;
    return true;
}

void Modem::disableHTTPConsoleMode() {
    SPDLOG_LOGGER_INFO(modemLogger, "HTTP console mode disabled");
    consoleMode.enabled = false;
    serial.write(ATE1"\r\n");
}

void Modem::sendATConsoleCommand(const QString &command) {
    SPDLOG_LOGGER_INFO(modemLogger, "sendConsoleCommand: {}", command.toStdString());
    serial.write((command.toStdString() + "\r\n").c_str());
}

void Modem::sendUSSDConsoleCommand(const QString &command) {
    SPDLOG_LOGGER_INFO(modemLogger, "sendUSSDConsoleCommand: {}", command.toStdString());
    serial.write((AT_CUSD"=1,\"" + command.toStdString() + "\",15\r\n").c_str());
}

void Modem::sendHTTPConsoleCommand(const QString &command, httpMethod_t method) {
    SPDLOG_LOGGER_INFO(modemLogger, "sendHTTPConsoleCommand: {}", command.toStdString());

    if (method == httpMethod_t::HM_GET) {
        serial.write(("AT+HTTPGET=\"" + command.toStdString() + "\"\r\n").c_str());
    } else if (method == httpMethod_t::HM_POST) {
        serial.write(("AT+HTTPPOST=\"" + command.toStdString() + "\"\r\n").c_str());
    }
}

bool Modem::checkAT() {
    auto command = GetCommand(AT, serial);
    auto response = command.execute(false);

    if (response.indexOf("OK") != -1) {
        return true;
    }

    return false;
}

bool Modem::checkRegistration() {
    auto command = GetCommand(AT_CREG"?", serial);
    auto response = command.execute(false);

    if (response.indexOf("+CREG: 0,1") != -1
        || (response.indexOf("+CREG: 0,5") != -1)
        || (response.indexOf("+CREG: 1,1") != -1)
        || (response.indexOf("+CREG: 1,5") != -1)) {
        return true;
    }

    return false;
}

bool Modem::call(const std::string &number) {
    Task task(ATD + number + ";", serial);
    SPDLOG_LOGGER_INFO(modemLogger, "Calling {}", number);
    commRes_t res = task.execute();

    if (res == commRes::CR_OK) {
        SPDLOG_LOGGER_INFO(modemLogger, "Call to {} was successful", number);
        currentCall.callDirection = callDirection::CD_OUTGOING;
        currentCall.callResult = callResult::CR_NO_ANSWER;
        currentCall.number = QString::fromStdString(number);
        currentCall.startTime = QDateTime::currentDateTime();
        return true;
    } else {
        SPDLOG_LOGGER_INFO(modemLogger, "Call to {} failed", number);
        return false;
    }
}

bool Modem::hangUp() {
    Task task(ATH, serial);
    SPDLOG_LOGGER_INFO(modemLogger, "Hanging up with {}", currentCall.number.toStdString());
    commRes_t res = task.execute();

    if (res == commRes::CR_OK) {
        SPDLOG_LOGGER_INFO(modemLogger, "Hanging up was successful");
        currentCall.endTime = QDateTime::currentDateTime();
        CacheManager::saveCall(currentCall);
        return true;
    } else {
        SPDLOG_LOGGER_WARN(modemLogger, "Hanging up failed");
        return false;
    }
}

bool Modem::answer() {
    Task task(ATA, serial);
    SPDLOG_LOGGER_INFO(modemLogger, "Answering {}", currentCall.number.toStdString());
    commRes_t res = task.execute();

    if (res == commRes::CR_OK) {
        SPDLOG_LOGGER_INFO(modemLogger, "Answering was successful");
        currentCall.callResult = callResult::CR_ANSWERED;
        return true;
    } else {
        SPDLOG_LOGGER_WARN(modemLogger, "Answering failed");
        return false;
    }
}

bool Modem::message(const std::string &number, const std::string &message) {
    SPDLOG_LOGGER_INFO(modemLogger, "Sending message to {}. Message: {}", number, message);
    auto setMessage = GetCommand(AT_CMGS"=\"" + number + "\"", serial);
    auto response = setMessage.execute(true, false);

    if (response.indexOf(">") == -1) {
        printColored(RED_PAIR, "Message sending failed. No > prompt received.");
        SPDLOG_LOGGER_WARN(modemLogger, "Error sending message. No > prompt received.");
        return false;
    }

    SPDLOG_LOGGER_INFO(modemLogger, "Received >. Writing message");
    Task task(message + char(26), serial);  // 26 is Ctrl+Z
    commRes_t res = task.execute(false);

    if (res == commRes::CR_OK) {
        printColored(GREEN_PAIR, "Message sent successfully");
        SPDLOG_LOGGER_INFO(modemLogger, "Message sent");

        CacheManager::saveMessage(Message(QString::fromStdString(number),
                                          QDateTime::currentDateTime().toString("yyyy/MM/dd,hh:mm:ss+02"),
                                          QString::fromStdString(message),
                                          messageDirection::MD_OUTGOING));
        return true;
    }

    return false;
}

bool Modem::setMessageMode(bool mode) {
    SPDLOG_LOGGER_INFO(modemLogger, "Setting message mode to {}", mode);
    int modeInt = mode ? 1 : 0;
    auto command = SetCommand(AT_CMGF"=" + std::to_string(modeInt), serial);
    commRes_t result = command.execute();
    if (result == commRes::CR_OK) {
        return true;
    }
    return false;

}

bool Modem::setNumberID(bool mode) {
    SPDLOG_LOGGER_INFO(modemLogger, "Setting number ID to {}", mode);
    int modeInt = mode ? 1 : 0;
    auto command = SetCommand("AT+CLIP="+std::to_string(modeInt), serial);
    commRes_t result = command.execute();
    if (result == commRes::CR_OK) {
        return true;
    }
    return false;
}

bool Modem::setEchoMode(bool mode) {
    SPDLOG_LOGGER_INFO(modemLogger, "Setting echo to {}", mode);
    int modeInt = mode ? 1 : 0;
    auto command = SetCommand("ATE"+std::to_string(modeInt), serial);
    commRes_t result = command.execute();
    if (result == commRes::CR_OK) {
        return true;
    }
    return false;
}

QString Modem::aboutDevice() {
    SPDLOG_LOGGER_INFO(modemLogger, "Getting device info");
    auto command = GetCommand("ATI", serial);
    QString result = command.execute();
    result.remove("OK");
    return result;
}

void Modem::worker() {
    Modem::workerStatus = true;
    SPDLOG_LOGGER_INFO(modemLogger, "Worker started");
    Modem::listen();
}

void Modem::listen() {
    while (Modem::workerStatus) {
        if (serial.interruptDataRead) {
            dataInterruptHandler();
        }

        if (consoleMode.enabled) {
            if (consoleMode.consoleType == consoleType::CM_AT) {
                atConsoleMode();
            }

            if (consoleMode.consoleType == consoleType::CM_USSD) {
                ussdConsoleMode();
            }

            if (consoleMode.consoleType == consoleType::CM_HTTP) {
                httpConsoleMode();
            }

            continue;
        }

        QByteArray data = readLine();
        QString parsedLine = parseLine(data);

        if (parsedLine.contains("RING")) {
            SPDLOG_LOGGER_INFO(modemLogger, "RING received");
            ringHandler(parsedLine);
        }

        if (parsedLine.contains("CIEV: \"CALL\",1")) {
            SPDLOG_LOGGER_INFO(modemLogger, "CIEV: \"CALL\",1 received");
            cievCall1Handler();
        }

        if (parsedLine.contains("+CIEV: \"SOUNDER\",0")) {
            SPDLOG_LOGGER_INFO(modemLogger, "+CIEV: \"SOUNDER\",0 received");
            sounder0Handler();
        }

        if (parsedLine.contains("CIEV: \"CALL\",0")) {
            SPDLOG_LOGGER_INFO(modemLogger, "CIEV: \"CALL\",0 received");
            cievCall0Handler(parsedLine);
        }

        if (parsedLine.contains("CIEV: \"MESSAGE\",1")) {
            SPDLOG_LOGGER_INFO(modemLogger, "CIEV: \"MESSAGE\",1 received");
            message1Handler(parsedLine);
        }
    }
}
