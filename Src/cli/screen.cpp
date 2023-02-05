//
// Created by paul on 1/15/23.
//

#include "cli/cli.hpp"
#include "cli/definitions/colors.hpp"
#include "modem/utils/cache_manager.hpp"
#include "cli/modem_controller.hpp"
#include <cmath>

Screen::Screen(QString name, QSharedPointer<Screen> parentScreen) : screenName(std::move(name)),
                                                                     parentScreen(std::move(parentScreen)) {}

void Screen::render() {
    int activeOptionIndex = getActiveOption();

    int optionsPerPage = getMaxOptionsPerPage();

    int pagesCount = getPagesCount();
    int activePage = getActivePage();
    int activeOptionOnPage = activeOptionIndex % optionsPerPage;

    int startOptionIndex = activePage * optionsPerPage;

    if (pagesCount > 1) {
        clear();
        int previousLine = getcury(stdscr);
        move(LINES - 1, 0);
        printColored(FILLED_WHITE_PAIR, "Page " + QString::number(activePage + 1)
                                        + " of " + QString::number(pagesCount));
        move(previousLine, 0);
    }

    printColored(WHITE_PAIR, screenName, true, true);

    for (const auto &notification: notifications) {
        printColored(WHITE_PAIR, notification);
    }

    for (int i = 0; i < optionsPerPage; ++i) {
        int optionIndex = startOptionIndex + i;

        if (optionIndex >= static_cast<int>(screenOptions.size())) {
            break;
        }

        auto option = screenOptions[optionIndex];

        int color = (i == (activeOptionOnPage)) ? FILLED_WHITE_PAIR : WHITE_PAIR;

        if (i == 0 && i == activeOptionOnPage && activePage == 0) {
            color = FILLED_RED_PAIR;
        }
        if (option->isSwitcher) {
            if (i == activeOptionOnPage) {
                color = option->switcher ? FILLED_GREEN_PAIR : FILLED_RED_PAIR;
            } else {
                color = option->switcher ? GREEN_PAIR : RED_PAIR;
            }
        }
        if (!option->isAvailable) {
            color = (i == (activeOptionOnPage)) ? FILLED_RED_PAIR : RED_PAIR;
        }

        printColored(color, option->optionName);
    }

    refresh();
}

void Screen::addScreenOption(const QString &name, std::function<void()> const &action) {
    auto option = QSharedPointer<Option>::create(name, action);
    screenOptions.push_back(option);
    optionsMap[name] = option;
}

void Screen::addScreenOption(const QString &name, std::function<void()> const &action, bool switcher) {
    auto option = QSharedPointer<Option>::create(name, action, switcher);
    screenOptions.push_back(option);
    optionsMap[name] = option;
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

ContactScreen::ContactScreen(QSharedPointer<Screen> parentScreen, const Contact &contact, CLI &cli) : Screen(
        contact.name + ": " + contact.number, std::move(parentScreen)), contact(contact) {
    addScreenOption("Back", [&cli]() {
        cli.gotoParentScreen();
    });
    addScreenOption("Call", [&cli, &contact]() {
        cli.modemController->call(contact.number);
    });
    addScreenOption("SMS", [&cli, &contact]() {
        cli.modemController->sendMessage(contact.number);
    });
    addScreenOption("Delete", [&cli, &contact]() {
        CacheManager::removeContact(contact.name);

        printColored(GREEN_PAIR, "Contact deleted. Press any key to continue...");
        getch();
        cli.viewContacts();
    });
    addScreenOption("Edit", [&cli, &contact]() {
        printColored(YELLOW_PAIR, "Enter new name: ");
        QString newName = readString();
        printColored(YELLOW_PAIR, "Enter new number: ");
        QString newNumber = readString();

        CacheManager::removeContact(contact.name);
        CacheManager::addContact(newName, newNumber);

        printColored(GREEN_PAIR, "Contact updated. Press any key to continue...");
        getch();
        cli.viewContacts();
    });
}
