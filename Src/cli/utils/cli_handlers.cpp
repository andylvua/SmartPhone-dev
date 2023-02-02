//
// Created by Andrew Yaroshevych on 01.02.2023.
//

#include "cli/cli.hpp"
#include "modem/utils/cache_manager.hpp"

void CLI::handleIncomingCall(const QString &number) {
    auto contact = CacheManager::getContact(number.toStdString());

    QString info = "Incoming call from ";

    if (contact.hasValue()) {
        info += contact.name;
    } else {
        info += number;
    }

    CLI::screenMap["Incoming Call"]->addNotification(info);
    CLI::screenMap["In Call"]->addNotification(info);
    changeScreen("Incoming Call");
}

void CLI::handleIncomingSMS() {
    CLI::screenMap["Main"]->addNotification("    *New SMS");
    renderScreen();
}

void CLI::handleCallEnded() {
    CLI::screenMap["Incoming Call"]->notifications.clear();
    CLI::screenMap["In Call"]->notifications.clear();
    changeScreen("Main");
}
