//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#ifndef PHONE_CACHE_MANAGER_HPP
#define PHONE_CACHE_MANAGER_HPP

#include <string>
#include <QString>
#include "../../Inc/modem/media_types.hpp"

constexpr const char *CONTACTS_FILEPATH = "../module_cache/contacts.txt";
constexpr const char *MESSAGES_FILEPATH = "../module_cache/messages.txt";
constexpr const char *CALLS_FILEPATH = "../module_cache/calls.txt";
#define LOGS_FILEPATH "../logs/log.txt";

class CacheManager {
public:

    static void saveMessage(const Message &message);

    static void saveCall(const Call &call);

    static void addContact(const std::string &name, const std::string &number);

    static void removeContact(const std::string &name);

    static void removeNewMessageNotification();

    static std::vector<Contact> getContacts();

    static Contact getContact(const std::string &info);

    static void listMessages();

    static void listCalls();

    static void checkCacheFiles();

    static void listLogs();

private:

    static void writeToFile(const std::string &fileName, const std::string &data);

};

#endif //PHONE_CACHE_MANAGER_HPP
