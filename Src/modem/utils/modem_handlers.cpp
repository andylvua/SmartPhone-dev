//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#include "modem/modem.hpp"
#include "logging.hpp"
#include "modem/utils/cache_manager.hpp"
#include "cli/utils/io/ncurses_io.hpp"
#include "cli/defenitions/colors.hpp"

const auto modemLogger = spdlog::get("modem");

void Modem::dataInterruptHandler() {
    QByteArray data;
    SPDLOG_LOGGER_INFO(modemLogger, "Data interrupt handler started");
    serial.buffer.clear();
    if (serial.waitForReadyRead(serial.timeout)) {
        data = serial.readAll();
        while (serial.waitForReadyRead(serial.timeout))
            data += serial.readAll();
    }

    serial.buffer = data;
    SPDLOG_LOGGER_INFO(modemLogger, "Buffering data: {}\nExiting data interrupt handler", data.toStdString());
    serial.interruptDataRead = false;
}

void Modem::ringHandler(const QString &parsedLine) {
    if (parsedLine.contains("CLIP")) {
        QString number = parsedLine.split("\"")[1];
        currentCall = Call{};
        currentCall.callDirection = callDirection::CD_INCOMING;
        currentCall.number = number;
        currentCall.startTime = QDateTime::currentDateTime();
        currentCall.callResult = callResult::CR_NO_ANSWER;
        SPDLOG_LOGGER_INFO(modemLogger, "Incoming call from: {}", number.toStdString());
        emit incomingCall(number);
    }
}

void Modem::cievCall0Handler(const QString &parsedLine) {
    currentCall.endTime = QDateTime::currentDateTime();
    SPDLOG_LOGGER_INFO(modemLogger, "CALL 0 received. Saving call");
    if (parsedLine.contains("BUSY")) {
        currentCall.callResult = callResult::CR_NO_ANSWER;
        printColored(YELLOW_PAIR, "Call ended: BUSY");
        SPDLOG_LOGGER_INFO(modemLogger, "Call busy");
    }

    CacheManager::saveCall(currentCall);
    SPDLOG_LOGGER_INFO(modemLogger, "Call saved");
    printColored(YELLOW_PAIR, "Call ended");
    QThread::msleep(1000);
    emit callEnded();
    SPDLOG_LOGGER_INFO(modemLogger, "Call ended signal emitted");
}


void Modem::cievCall1Handler() {
    if (currentCall.callDirection == callDirection::CD_INCOMING) {
        currentCall.callResult = callResult::CR_ANSWERED;
        printColored(GREEN_PAIR, "Call answered");
        SPDLOG_LOGGER_INFO(modemLogger, "Call answered");
    }
}

void Modem::sounder0Handler() {
    if (currentCall.callDirection == callDirection::CD_OUTGOING) {
        currentCall.callResult = callResult::CR_ANSWERED;
        printColored(YELLOW_PAIR, "Call answered");
        SPDLOG_LOGGER_INFO(modemLogger, "Call answered/declined");
    }
}

void Modem::message1Handler(const QString &parsedLine) {
    if (parsedLine.contains("CMT")) {
        QString number = parsedLine.split("\"")[3];
        QString dateTime = parsedLine.split("\"")[5];
        QString message = parsedLine.split("\"")[6];

        CacheManager::saveMessage(Message(number,
                                          dateTime,
                                          message,
                                          messageDirection::MD_INCOMING));
        printColored(YELLOW_PAIR, ("New message from " + number + ": " + message).toStdString());
        SPDLOG_LOGGER_INFO(modemLogger, "New message from: {} Message: {} Date: {}", number.toStdString(),
                           message.toStdString(), dateTime.toStdString());
        emit incomingSMS();
    }
}
