//
// Created by paul on 1/15/23.
//

#include <utility>
#include "../../Inc/cli/cli.hpp"
#include "../../Inc/cli/colors.hpp"
#include "../../Inc/modem/cache_manager.hpp"

Screen::Screen(QString name, std::shared_ptr<Screen> parentScreen) : screenName(std::move(name)),
                                                                     parentScreen(std::move(parentScreen)) {}

void Screen::addScreenOption(const QString &name, std::function<void()> const& action) {
    Option option(name, action);
    screenOptions.push_back(option);
}

int Screen::getActiveOption() const {
    if (activeOption == -1) {
        return -1;
    }

    auto optionsSize = static_cast<int>(screenOptions.size());

    return activeOption % optionsSize;
}

void Screen::addNotification(const QString &notification) {
    notifications.push_back(notification);
}

void Screen::removeScreenOption(const QString &option) {
    screenOptions.erase(std::remove_if(screenOptions.begin(), screenOptions.end(),
                                       [&option](const Option &screenOption) {
                                           return screenOption.optionName == option;
                                       }), screenOptions.end());
}

void Screen::removeNotification(const QString &notification) {
    notifications.erase(std::remove(notifications.begin(), notifications.end(), notification), notifications.end());
}

void Screen::removeScreenOption(int index) {
    screenOptions.erase(screenOptions.begin() + index);
}

ContactScreen::ContactScreen(std::shared_ptr<Screen> parentScreen, const Contact& contact, CLI &cli) : Screen(
        contact.name + ": " + contact.number, std::move(parentScreen)), contact(contact) {
    addScreenOption("Back", [&cli]() {
        cli.gotoParentScreen();
    });
    addScreenOption("Call", [&cli, &contact]() {
        cli.call(contact.number);
    });
    addScreenOption("SMS", [&cli, &contact]() {
        cli.sendMessage(contact.number);
    });
    addScreenOption("Delete", [&cli, this, &contact]() {
        CacheManager::removeContact(contact.name.toStdString());

        printColored(GREEN_PAIR, "Contact deleted. Press any key to continue...");
        getch();
        cli.viewContacts();
    });
    addScreenOption("Edit", [&cli, this, &contact]() {
        printColored(YELLOW_PAIR, "Enter new name: ");
        std::string newName = readString();
        printColored(YELLOW_PAIR, "Enter new number: ");
        std::string newNumber = readString();

        CacheManager::removeContact(contact.name.toStdString());
        CacheManager::addContact(newName, newNumber);

        printColored(GREEN_PAIR, "Contact updated. Press any key to continue...");
        getch();
        cli.viewContacts();
    });
}