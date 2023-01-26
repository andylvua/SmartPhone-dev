//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#include "../../Inc/modem/modem.hpp"
#include "../../Inc/modem/cache_manager.hpp"
#include "../../Inc/cli/colors.hpp"
#include "../../Inc/logging.hpp"
#include "../../Inc/decoder.hpp"
#include "../../Inc/cli/ncurses_io.hpp"
#include "../../Inc/command/command.hpp"
#include "../../Inc/command/commands_list.hpp"

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
    commRes_t messageModeStatus = setMessageMode.execute(false);

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

void Modem::atConsoleMode() {
    while (consoleMode.enabled) {
        QByteArray data = readLine();
        QString parsedLine = parseLine(data);

        if (parsedLine.isEmpty()) {
            return;
        }
        SPDLOG_LOGGER_INFO(modemLogger, "Console mode: {}", parsedLine.toStdString());
        if (parsedLine.contains("ERROR")) {
            printColored(RED_PAIR, parsedLine.toStdString(), true, false, consoleMode.consoleWindow);
        } else if (parsedLine.contains("OK")) {
            printColored(GREEN_PAIR, parsedLine.toStdString(), true, false, consoleMode.consoleWindow);
        } else {
            printColored(WHITE_PAIR, parsedLine.toStdString(), true, false, consoleMode.consoleWindow);
        }
    }
}

void Modem::ussdConsoleMode() {
    while (consoleMode.enabled) {
        QByteArray data = readLine();
        QString parsedLine = parseLine(data);

        if (parsedLine.isEmpty()) {
            return;
        }
        SPDLOG_LOGGER_INFO(modemLogger, "Console mode: {}", parsedLine.toStdString());

        ussdEncoding encoding;

        if (parsedLine.right(3) == ",15") {
            encoding = ussdEncoding::UE_GSM7;
        } else if (parsedLine.right(3) == ",72") {
            encoding = ussdEncoding::UE_UCS2;
        } else {
            encoding = ussdEncoding::UE_UNKNOWN;
        }

        if (encoding == ussdEncoding::UE_UNKNOWN) {
            printColored(RED_PAIR, parsedLine.toStdString(), true, false, consoleMode.consoleWindow);
            continue;
        }

        auto response = parsedLine.split("\"")[1];

        if (encoding == ussdEncoding::UE_GSM7) {
            QString decoded = Decoder::decode7Bit(response);
            printColored(GREEN_PAIR, decoded.toStdString(), true, false, consoleMode.consoleWindow);
        } else {
            QString decoded = Decoder::decodeUCS2(response);
            printColored(GREEN_PAIR, decoded.toStdString(), true, false, consoleMode.consoleWindow);
        }
    }
}