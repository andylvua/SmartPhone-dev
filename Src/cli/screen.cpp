//
// Created by paul on 1/15/23.
//

#include <utility>
#include "cli/cli.hpp"
#include "cli/definitions/colors.hpp"
#include "modem/utils/cache_manager.hpp"
#include <cmath>

Screen::Screen(QString name, std::shared_ptr<Screen> parentScreen) : screenName(std::move(name)),
                                                                     parentScreen(std::move(parentScreen)) {}

void Screen::addScreenOption(const QString &name, std::function<void()> const &action) {
    std::shared_ptr option = std::make_shared<Option>(name, action);
    screenOptions.push_back(option);
}

void Screen::addScreenOption(const QString &name, std::function<void()> const &action, bool switcher) {
    std::shared_ptr option = std::make_shared<Option>(name, action, true, switcher);
    screenOptions.push_back(option);
}

int Screen::getActiveOption() const {
    if (activeOption == -1) {
        return -1;
    }

    auto optionsSize = static_cast<int>(screenOptions.size());

    return activeOption % optionsSize;
}

int Screen::getPagesCount() const {
    return static_cast<int>(std::ceil(static_cast<double>(screenOptions.size()) / getMaxOptionsPerPage()));
}

int Screen::getActivePage() const {
    if (activeOption == -1) {
        return 0;
    }

    return static_cast<int>(std::floor(static_cast<double>(getActiveOption()) / getMaxOptionsPerPage()));
}

int Screen::getMaxOptionsPerPage() const {
    if (screenOptions.size() == (LINES - 1 - notifications.size())) {
        return static_cast<int>(screenOptions.size());
    }

    return static_cast<int>(LINES - 2 - notifications.size());
}

bool Screen::isFirstPage() const {
    return getActivePage() == 0;
}

bool Screen::isLastPage() const {
    return getActivePage() + 1 == getPagesCount();
}

void Screen::addNotification(const QString &notification) {
    notifications.push_back(notification);
}

void Screen::removeScreenOption(int index) {
    screenOptions.erase(screenOptions.begin() + index);
}

ContactScreen::ContactScreen(std::shared_ptr<Screen> parentScreen, const Contact &contact, CLI &cli) : Screen(
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
    addScreenOption("Delete", [&cli, &contact]() {
        CacheManager::removeContact(contact.name.toStdString());

        printColored(GREEN_PAIR, "Contact deleted. Press any key to continue...");
        getch();
        cli.viewContacts();
    });
    addScreenOption("Edit", [&cli, &contact]() {
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
