//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#include "../../Inc/modem/cache_manager.hpp"
#include "../../Inc/logging.hpp"
#include "../../Inc/cli/ncurses_io.hpp"
#include "../../Inc/cli/ncurses_utils.hpp"
#include <fstream>

const auto cacheLogger = spdlog::basic_logger_mt("cache", "../logs/log.txt", true);

void CacheManager::checkCacheFiles() {
    std::ifstream messagesFile(MESSAGES_FILEPATH);
    SPDLOG_LOGGER_INFO(cacheLogger, "Checking messages file...");
    if (!messagesFile.is_open()) {
        std::ofstream createMessagesFile(MESSAGES_FILEPATH);
        createMessagesFile.close();
    }
    SPDLOG_LOGGER_INFO(cacheLogger, "Messages file OK");

    std::ifstream callsFile(CALLS_FILEPATH);
    SPDLOG_LOGGER_INFO(cacheLogger, "Checking calls file...");
    if (!callsFile.is_open()) {
        std::ofstream createCallsFile(CALLS_FILEPATH);
        createCallsFile.close();
    }
    SPDLOG_LOGGER_INFO(cacheLogger, "Calls file OK");

    std::ifstream contactsFile(CONTACTS_FILEPATH);
    SPDLOG_LOGGER_INFO(cacheLogger, "Checking contacts file...");
    if (!contactsFile.is_open()) {
        std::ofstream createContactsFile(CONTACTS_FILEPATH);
        createContactsFile.close();
    }
    SPDLOG_LOGGER_INFO(cacheLogger, "Contacts file OK");
}

void CacheManager::writeToFile(const std::string &fileName, const std::string &data) {
    std::ifstream checkFile(fileName);
    if (!checkFile.is_open()) {
        std::ofstream createFile(fileName);
        createFile.close();
    }

    std::ofstream file(fileName, std::ios::app);
    file << data << std::endl;
    file.close();
}

void CacheManager::saveMessage(const Message &message) {
    std::string isIncoming = message.messageDirection == messageDirection::MD_INCOMING ? "INCOMING" : "OUTGOING";
    QString dateTime = message.dateTime;
    std::string data = message.number.toStdString() + "; " + isIncoming + "; " + dateTime.toStdString() + "; "
                       + message.message.toStdString();

    if (message.messageDirection == messageDirection::MD_INCOMING) {
        data += " * new *";
    }

    SPDLOG_LOGGER_INFO(cacheLogger, "Saving message: {}", data);
    writeToFile(MESSAGES_FILEPATH, data);
}

void CacheManager::saveCall(const Call &call) {
    QString dateTime = call.startTime.toString("dd.MM.yyyy hh:mm:ss");
    QString duration = QString::number(call.startTime.secsTo(call.endTime));
    std::string callDirection = call.callDirection == callDirection::CD_INCOMING ? "INCOMING" : "OUTGOING";
    std::string isMissed = call.callResult == callResult::CR_NO_ANSWER ? "MISSED" : "ACCEPTED";

    std::string data = call.number.toStdString() + "; " + dateTime.toStdString() + "; "
                       + duration.toStdString() + "; " + callDirection + "; " + isMissed;

    SPDLOG_LOGGER_INFO(cacheLogger, "Saving call: {}", data);
    writeToFile(CALLS_FILEPATH, data);
}

void CacheManager::addContact(const std::string &name, const std::string &number) {
    std::string data = name + "; " + number;
    SPDLOG_LOGGER_INFO(cacheLogger, "Adding contact: {}", data);
    writeToFile(CONTACTS_FILEPATH, data);
    SPDLOG_LOGGER_INFO(cacheLogger, "Contact added");
}

void CacheManager::removeContact(const std::string &name) {
    SPDLOG_LOGGER_INFO(cacheLogger, "Removing contact: {}", name);
    std::ifstream file(CONTACTS_FILEPATH);
    std::string line;
    std::string data;

    while (std::getline(file, line)) {
        if (line.find(name) == std::string::npos)
            data += line + "\n";
    }

    file.close();

    std::ofstream contactsFile(CONTACTS_FILEPATH);
    contactsFile << data;
    SPDLOG_LOGGER_INFO(cacheLogger, "Contact removed");
}

void CacheManager::removeNewMessageNotification() {
    std::ifstream file(MESSAGES_FILEPATH);
    std::string line;
    std::string data;

    while (std::getline(file, line)) {
        if (line.find("* new *") == std::string::npos) {
            data += line + "\n";
        } else {
            data += line.replace(line.find("* new *"), 7, "") + "\n";
        }
    }

    file.close();

    std::ofstream messagesFile(MESSAGES_FILEPATH);
    messagesFile << data;
}

std::vector<Contact> CacheManager::getContacts() {
    SPDLOG_LOGGER_INFO(cacheLogger, "Getting contacts");
    std::ifstream file(CONTACTS_FILEPATH);
    std::string line;
    std::vector<Contact> contacts;

    while (std::getline(file, line)) {
        auto contact = QString::fromStdString(line).split("; ");
        contacts.emplace_back(contact[0], contact[1]);
    }

    file.close();
    return contacts;
}

Contact CacheManager::getContact(const std::string &info) {
    SPDLOG_LOGGER_INFO(cacheLogger, "Getting contact by name: {}", info);
    std::ifstream file(CONTACTS_FILEPATH);
    std::string line;

    while (std::getline(file, line)) {
        if (line.find(info) != std::string::npos) {
            auto contact = QString::fromStdString(line).split("; ");
            return {contact[0], contact[1]};
        }
    }

    file.close();
    return {};
}

void CacheManager::listMessages() {
    SPDLOG_LOGGER_INFO(cacheLogger, "Listing messages");
    std::ifstream file(MESSAGES_FILEPATH);
    std::string line;
    std::string data;

    while (std::getline(file, line)) {
        auto message = QString::fromStdString(line).split("; ");
        data += std::string("Number: " + message[0].toStdString() + "\n"
                            + " Direction: " + message[1].toStdString() + "\n"
                            + " Date: " + message[2].toStdString() + "\n"
                            + " Message: " + message[3].toStdString()
                            + "\n");
    }

    displayPad(data, "Viewing messages");

    removeNewMessageNotification();
}

void CacheManager::listCalls() {
    SPDLOG_LOGGER_INFO(cacheLogger, "Listing calls");
    std::ifstream file(CALLS_FILEPATH);
    std::string line;
    std::string data;

    while (std::getline(file, line)) {
        auto call = QString::fromStdString(line).split("; ");
        data += std::string("Number: " + call[0].toStdString() + "\n"
                            + " Date: " + call[1].toStdString() + "\n"
                            + " Duration: " + call[2].toStdString() + "\n"
                            + " Direction: " + call[3].toStdString() + "\n"
                            + " Call result: " + call[4].toStdString()
                            + "\n");
    }

    displayPad(data, "Viewing calls");

    file.close();
}

void CacheManager::listLogs() {
    std::string command = "open " LOGS_FILEPATH;
    system(command.c_str());
}
