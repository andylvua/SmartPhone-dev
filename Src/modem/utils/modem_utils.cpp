//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#include "modem/modem.hpp"
#include "modem/utils/cache_manager.hpp"
#include "cli/definitions/colors.hpp"
#include "logging.hpp"
#include "cli/utils/ncurses/ncurses_io.hpp"
#include "modem/command/command.hpp"
#include "modem/command/commands_list.hpp"

const auto modemLogger = spdlog::get("modem");

QByteArray Modem::readLine() {
    QByteArray data;

    while (serial.waitForReadyRead(100)) {
        if (serial.bytesAvailable())
            data += serial.readAll();
    }

    return data;
}

QString Modem::parseLine(const QByteArray &line) {
    auto lineString = QString(line);
    if (lineString.isEmpty())
        return lineString;
    QString parsedLine = lineString.replace("\r\n\r\n", "\n").replace("\r\n", "");
    return parsedLine;
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

bool Modem::initialize() {
    CacheManager::checkCacheFiles();

    std::cout << YELLOW_COLOR << "Checking modem..." << RESET;
    outStream.flush();

    bool echoOk = enableEcho();

    if (!echoOk) {
        std::cout << RED_COLOR << "\nError: Failed to enable echo" << RESET << std::endl;
        SPDLOG_LOGGER_ERROR(modemLogger, "Failed to enable echo");

        int retries = 3;

        while (!echoOk && retries > 0) {
            std::cout << YELLOW_COLOR << "Retrying..." << RESET << std::endl;
            SPDLOG_LOGGER_INFO(modemLogger, "Retrying...");
            echoOk = enableEcho();
            retries--;
        }

        if (!echoOk) {
            std::cout << RED_COLOR << "Too many retries. Aborting" << RESET << std::endl;
            SPDLOG_LOGGER_ERROR(modemLogger, "Too many retries. Aborting");

            return false;
        }
    }

    std::cout << "\r";
    std::cout << GREEN_COLOR << "Performing modem initialization. Please wait" << RESET;
    outStream.flush();

    SPDLOG_LOGGER_INFO(modemLogger, "Checking AT...");
    bool atStatus = checkAT();
    if (!atStatus) {
        std::cout << RED_COLOR << "\nError: AT command failed" << RESET << std::endl;
        SPDLOG_LOGGER_ERROR(modemLogger, "AT command failed");
        return false;
    }

    std::cout << GREEN_COLOR << "." << RESET;
    outStream.flush();

    SPDLOG_LOGGER_INFO(modemLogger, "AT OK");

    SPDLOG_LOGGER_INFO(modemLogger, "Setting SMS mode to text...");
    auto setMessageMode = SetCommand(AT_CMGF"=1", serial);
    commRes_t messageModeStatus = setMessageMode.execute(workerStatus);

    if (messageModeStatus != commRes::CR_OK) {
        std::cout << RED_COLOR << "\nError: Message mode failed" << RESET << std::endl;
        SPDLOG_LOGGER_ERROR(modemLogger, "Message mode failed");
        return false;
    }

    std::cout << GREEN_COLOR << "." << RESET;
    outStream.flush();

    SPDLOG_LOGGER_INFO(modemLogger, "SMS mode OK");

    SPDLOG_LOGGER_INFO(modemLogger, "Checking registration...");
    bool registrationStatus = checkRegistration();
    if (!registrationStatus) {
        std::cout << RED_COLOR << "\nError: Registration failed. Check SIM card" << RESET << std::endl;
        SPDLOG_LOGGER_ERROR(modemLogger, "Registration failed");
        return false;
    }
    SPDLOG_LOGGER_INFO(modemLogger, "Registration OK");

    std::cout << GREEN_COLOR << "." << RESET;
    outStream.flush();

    SPDLOG_LOGGER_INFO(modemLogger, "Setting number identification...");
    auto setNumberIDTrue = SetCommand("AT+CLIP=1", serial);
    commRes_t numberIdentifierStatus = setNumberIDTrue.execute(false);
    if (numberIdentifierStatus != commRes::CR_OK) {
        std::cout << RED_COLOR << "\nError: Number identification failed" << RESET << std::endl;
        SPDLOG_LOGGER_ERROR(modemLogger, "Number identification failed");
        return false;
    }
    SPDLOG_LOGGER_INFO(modemLogger, "Number identification OK");

    std::cout << GREEN_COLOR << " ✓" << RESET;
    outStream.flush();

    QThread::msleep(300);
    SPDLOG_LOGGER_INFO(modemLogger, "Modem initialized successfully");
    return true;
}
