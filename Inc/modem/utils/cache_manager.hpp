//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#ifndef PHONE_CACHE_MANAGER_HPP
#define PHONE_CACHE_MANAGER_HPP

#include <string>
#include <QString>
#include "modem/media_types.hpp"

constexpr const char *CONTACTS_FILEPATH = "../module_cache/contacts.txt";
constexpr const char *MESSAGES_FILEPATH = "../module_cache/messages.txt";
constexpr const char *CALLS_FILEPATH = "../module_cache/calls.txt";
#define LOGS_FILEPATH "../logs/log.txt"

class CacheManager {
public:

    static void saveMessage(const Message &message);

    static void saveCall(const Call &call);

    static void addContact(const QString &name, const QString &number);

    static void removeContact(const QString &name);

    static void removeNewMessageNotification();

    static std::vector<Contact> getContacts();

    static Contact getContact(const QString &info);

    static void listMessages();

    static void listCalls();

    static void checkCacheFiles();

    static void listLogs();

private:

    static void writeToFile(const QString &fileName, const QString &data);

};

#endif //PHONE_CACHE_MANAGER_HPP
