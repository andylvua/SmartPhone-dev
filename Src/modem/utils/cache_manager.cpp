//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#include "modem/utils/cache_manager.hpp"
#include "logging.hpp"
#include "cli/utils/ncurses/ncurses_utils.hpp"
#include <QTextStream>


#define SEMICOLON_SEPARATOR(...) QStringList{__VA_ARGS__}.join("; ")

const auto cacheLogger = spdlog::basic_logger_mt("cache",
                                                 LOGS_FILEPATH, true);

void createIfNotExists(const QString &fileName) {
    SPDLOG_LOGGER_INFO(cacheLogger, "Checking file: {}", fileName.toStdString());
    QFile file(fileName);

    if (!file.exists()) {
        SPDLOG_LOGGER_INFO(cacheLogger, "File {} does not exist, creating", fileName.toStdString());
        file.open(QIODevice::WriteOnly);
        file.close();
    }
}

void CacheManager::checkCacheFiles() {
    createIfNotExists(MESSAGES_FILEPATH);
    createIfNotExists(CALLS_FILEPATH);
    createIfNotExists(CONTACTS_FILEPATH);
}

void CacheManager::writeToFile(const QString &fileName, const QString &data) {
    QFile outputFile(fileName);
    if (!outputFile.exists()) {
        outputFile.open(QIODevice::WriteOnly);
        outputFile.close();
    }

    outputFile.open(QIODevice::Append);
    QTextStream out(&outputFile);
    out << data << Qt::endl;
    outputFile.close();
}

void CacheManager::saveMessage(const Message &message) {
    QString isIncoming = message.messageDirection == messageDirection::MD_INCOMING ? "INCOMING" : "OUTGOING";
    QString dateTime = message.dateTime;
    QString data = SEMICOLON_SEPARATOR(message.number, isIncoming, dateTime, message.message);

    if (message.messageDirection == messageDirection::MD_INCOMING) {
        data += " * new *";
    }

    SPDLOG_LOGGER_INFO(cacheLogger, "Saving message: {}", data.toStdString());
    writeToFile(MESSAGES_FILEPATH, data);
}

void CacheManager::saveCall(const Call &call) {
    QString dateTime = call.startTime.toString("dd.MM.yyyy hh:mm:ss");
    QString duration = QString::number(call.startTime.secsTo(call.endTime));
    QString callDirection = call.callDirection == callDirection::CD_INCOMING ? "INCOMING" : "OUTGOING";
    QString isMissed = call.callResult == callResult::CR_NO_ANSWER ? "MISSED" : "ACCEPTED";

    QString data = SEMICOLON_SEPARATOR(call.number, dateTime, duration, callDirection, isMissed);

    SPDLOG_LOGGER_INFO(cacheLogger, "Saving call: {}", data.toStdString());
    writeToFile(CALLS_FILEPATH, data);
}

void CacheManager::addContact(const QString &name, const QString &number) {
    QString data = SEMICOLON_SEPARATOR(name, number);

    SPDLOG_LOGGER_INFO(cacheLogger, "Adding contact: {}", data.toStdString());

    writeToFile(CONTACTS_FILEPATH, data);
    SPDLOG_LOGGER_INFO(cacheLogger, "Contact added");
}

void CacheManager::removeContact(const QString &name) {
    SPDLOG_LOGGER_INFO(cacheLogger, "Removing contact: {}", name.toStdString());

    QFile file(CONTACTS_FILEPATH);
    file.open(QIODevice::ReadOnly);

    QString data;
    while (!file.atEnd()) {
        QString line = file.readLine();
        if (!line.contains(name)) {
            data += line + "\n";
        }
    }

    file.close();

    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);

    out << data;
    file.close();

    SPDLOG_LOGGER_INFO(cacheLogger, "Contact removed");
}

void CacheManager::removeNewMessageNotification() {
    QFile file(MESSAGES_FILEPATH);
    file.open(QIODevice::ReadOnly);

    QString data;
    while (!file.atEnd()) {
        QString line = file.readLine();
        if (line.contains("* new *")) {
            line.replace(line.indexOf("* new *"), 7, "");
        }
        data += line + "\n";
    }

    file.close();

    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);

    out << data;
    file.close();
}

QVector<Contact> CacheManager::getContacts() {
    SPDLOG_LOGGER_INFO(cacheLogger, "Getting contacts");
    QFile file(CONTACTS_FILEPATH);
    file.open(QIODevice::ReadOnly);
    QString line;

    QVector<Contact> contacts;

    while (!file.atEnd()) {
        line = file.readLine();
        auto contact = line.split("; ");
        contacts.push_back({contact[0], contact[1]});
    }

    file.close();
    return contacts;
}

Contact CacheManager::getContact(const QString &info) {
    SPDLOG_LOGGER_INFO(cacheLogger, "Getting contact by name: {}", info.toStdString());
    QFile file(CONTACTS_FILEPATH);
    file.open(QIODevice::ReadOnly);
    QString line;

    while (!file.atEnd()) {
        line = file.readLine();
        if (line.contains(info)) {
            auto contact = line.split("; ");
            return {contact[0], contact[1]};
        }
    }

    file.close();
    return {};
}

void CacheManager::listMessages() {
    SPDLOG_LOGGER_INFO(cacheLogger, "Listing messages");
    QFile file(MESSAGES_FILEPATH);
    file.open(QIODevice::ReadOnly);
    QString line;

    QString data;

    while (!file.atEnd()) {
        line = file.readLine();
        auto message = line.split("; ");
        data += "Number: "       + message[0] + "\n"
                + " Direction: " + message[1] + "\n"
                + " Date: "      + message[2] + "\n"
                + " Message: "   + message[3] + "\n";
    }

    NcursesUtils::displayPad(data, "Viewing messages");

    removeNewMessageNotification();
}

void CacheManager::listCalls() {
    SPDLOG_LOGGER_INFO(cacheLogger, "Listing calls");
    QFile file(CALLS_FILEPATH);
    file.open(QIODevice::ReadOnly);
    QString line;

    QString data;

    while (!file.atEnd()) {
        line = file.readLine();
        auto call = line.split("; ");
        data += "Number: "         + call[0] + "\n"
                + " Date: "        + call[1] + "\n"
                + " Duration: "    + call[2] + "\n"
                + " Direction: "   + call[3] + "\n"
                + " Call result: " + call[4] + "\n";
    }

    NcursesUtils::displayPad(data, "Viewing calls");

    file.close();
}

void CacheManager::listLogs() {
    QFile logFile(LOGS_FILEPATH);
    NcursesUtils::displayPad(logFile, "Viewing logs");
}
