//
// Created by Andrew Yaroshevych on 01.02.2023.
//

#include "cli/cli.hpp"
#include "cli/utils/cli_utils.hpp"
#include "cli/utils/ncurses_utils.hpp"
#include "cli/definitions/colors.hpp"
#include "modem/utils/cache_manager.hpp"

#ifdef BUILD_ON_RASPBERRY
#include "rotary_reader/rotary_dial.hpp"
RotaryDial rtx;
#endif

void render(const QSharedPointer<Screen> &screen) {
    int activeOptionIndex = screen->getActiveOption();

    int optionsPerPage = screen->getMaxOptionsPerPage();

    int pagesCount = screen->getPagesCount();
    int activePage = screen->getActivePage();
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

    printColored(WHITE_PAIR, screen->screenName, true, true);

    for (const auto &notification: screen->notifications) {
        printColored(WHITE_PAIR, notification);
    }

    for (int i = 0; i < optionsPerPage; ++i) {
        int optionIndex = startOptionIndex + i;

        if (optionIndex >= static_cast<int>(screen->screenOptions.size())) {
            break;
        }

        auto option = screen->screenOptions[optionIndex];

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

bool checkNumber(QString &number) {
    if (number[0] != '+') {
        number = '+' + number;
    }

    if (number.length() != 13) {
        printColored(RED_PAIR, "Invalid number: " + number);
        return false;
    }

    for (int i = 1; i < number.length(); ++i) {
        if (!number[i].isDigit()) {
            printColored(RED_PAIR, "Invalid number");
            return false;
        }
    }

    return true;
}

void CLI::disableNcursesScreen() {
    curs_set(1);
    NcursesUtils::releaseScreen();
    system("clear");
}

void CLI::enableNcursesScreen() {
    NcursesUtils::initScreen();
    renderScreen();
}

void CLI::rejectCall() {
    printColored(YELLOW_PAIR, "Rejecting call");
    modem.hangUp();
    CLI::screenMap["Incoming Call"]->notifications.clear();
    CLI::screenMap["In Call"]->notifications.clear();
    changeScreen("Main");
}

void CLI::answerCall() {
    printColored(YELLOW_PAIR, "Answering call");
    modem.answer();
    changeScreen("In Call");
}

void CLI::viewCallHistory() const {
    printColored(YELLOW_PAIR, "Call history:");
    CacheManager::listCalls();
    renderScreen();
}

void CLI::call() {
    QString number;
    printColored(YELLOW_PAIR, "Enter number");
#ifdef BUILD_ON_RASPBERRY
    printColored(YELLOW_PAIR, "Read from rotary dial or keyboard? (r/k)");
    QString input;
    input = readString();

    if (input == "r") {
        printColored(YELLOW_PAIR, "Reading from rotary dial");
        number = rtx.listen_for_number();
    } else if (input == "k") {
        printColored(YELLOW_PAIR, "Reading from keyboard");
        number = readString();
    } else {
        printColored(RED_PAIR, "Invalid input");
        return;
    }
#else
    number = readString();
#endif

    if (!checkNumber(number)) {
        return;
    }

    printColored(YELLOW_PAIR, "Calling...");
    modem.call(number);

    CLI::screenMap["In Call"]->addNotification("Calling " + number);
    changeScreen("In Call");
}

void CLI::call(const QString &number) {
    printColored(YELLOW_PAIR, "Calling...");
    modem.call(number);

    CLI::screenMap["In Call"]->addNotification("Calling " + number);
    changeScreen("In Call");
}

void CLI::hangUp() {
    modem.hangUp();
    CLI::screenMap["In Call"]->notifications.clear();

    printColored(YELLOW_PAIR, "Hanging up");
    QThread::msleep(2000);
    changeScreen("Main");
}

void CLI::addContact() {
    printColored(YELLOW_PAIR, "Adding contact");
    QString name;
    QString number;
    printColored(YELLOW_PAIR, "Enter name");
    name = readString();
    printColored(YELLOW_PAIR, "Enter number");
#ifdef BUILD_ON_RASPBERRY
    printColored(YELLOW_PAIR, "Read from rotary dial or keyboard? (r/k)");
    QString input;
    input = readString();

    if (input == "r") {
        printColored(YELLOW_PAIR, "Reading from rotary dial");
        number = rtx.listen_for_number();
    } else if (input == "k") {
        printColored(YELLOW_PAIR, "Reading from keyboard");
        number = readString();
    } else {
        printColored(RED_PAIR, "Invalid input");
        return;
    }
#else
    number = readString();
#endif
    if (!checkNumber(number)) {
        return;
    }

    CacheManager::addContact(name, number);
    printColored(GREEN_PAIR, "Contact added. Press any key to continue");
    getch();
    changeScreen("Contacts");
}

void CLI::viewContacts() {
    QVector<Contact> contacts = CacheManager::getContacts();
    auto contactsPage = CLI::screenMap["Contacts Page"];

    contactsPage->screenOptions.erase(
            contactsPage->screenOptions.begin() + 1,
            contactsPage->screenOptions.end());

    for (const auto &contact: contacts) {
        contactsPage->addScreenOption(
                contact.name + ": " + contact.number,
                [contact, this]() {
                    auto contactScreen = QSharedPointer<ContactScreen>::create(
                            CLI::screenMap["Contacts Page"],
                            contact,
                            *this);

                    currentScreen = contactScreen;
                    renderScreen();
                });
    }
    changeScreen("Contacts Page");
    renderScreen();
}

void CLI::viewMessages() {
    CLI::screenMap["Main"]->notifications.clear();
    printColored(YELLOW_PAIR, "Listing messages");
    CacheManager::listMessages();
    renderScreen();
}

void CLI::sendMessage() {
    QString number;
    QString message;
    printColored(YELLOW_PAIR, "Enter number: ");
#ifdef BUILD_ON_RASPBERRY
    printColored(YELLOW_PAIR, "Read from rotary dial or keyboard? (r/k)");
    QString input;
    input = readString();

    if (input == "r") {
        printColored(YELLOW_PAIR, "Reading from rotary dial");
        number = rtx.listen_for_number();
    } else if (input == "k") {
        printColored(YELLOW_PAIR, "Reading from keyboard");
        number = readString();
    } else {
        printColored(RED_PAIR, "Invalid input");
        return;
    }
#else
    number = readString();
#endif

    if (!checkNumber(number)) {
        return;
    }

    printColored(YELLOW_PAIR, "Enter message: ");
    message = readString();
    printColored(YELLOW_PAIR, "Sending SMS");

    modem.message(number, message);
}

void CLI::sendMessage(const QString &number) {
    QString message;
    printColored(YELLOW_PAIR, "Enter message: ");
    message = readString();
    printColored(YELLOW_PAIR, "Sending SMS");

    modem.message(number, message);
}

void CLI::viewLogs() const {
    printColored(GREEN_PAIR, "Opening logs file");
    CacheManager::listLogs();
    renderScreen();
}

void CLI::setMessageMode() {
    auto messageModeOption = CLI::screenMap["Debug Settings"]->optionsMap["Text Mode"];

    bool success = modem.setMessageMode(!messageModeOption->getState());
    if (success) {
        messageModeOption->switchState();
    } else {
        printColored(RED_PAIR, "Failed to set Text Mode");
    }
    updateScreen();
}

void CLI::setNumberID() {
    auto numberIDOption = CLI::screenMap["Debug Settings"]->optionsMap["Number Identifier"];

    bool success = modem.setNumberID(!numberIDOption->getState());
    if (success) {
        numberIDOption->switchState();
    } else {
        printColored(RED_PAIR, "Failed to set Number Identifier");
    }
    updateScreen();
}

void CLI::setEchoMode() {
    auto echoModeOption = CLI::screenMap["Debug Settings"]->optionsMap["Echo Mode"];

    bool success = modem.setEchoMode(!echoModeOption->getState());
    if (success) {
        echoModeOption->switchState();
    } else {
        printColored(RED_PAIR, "Failed to set Echo Mode");
    }
    updateScreen();
}

void CLI::aboutDevice() {
    changeScreen("About Device");
    printColored(YELLOW_PAIR, "Getting device info", false);
    QString aboutInfo = modem.aboutDevice();
    NcursesUtils::clearCurrentLine();
    printColored(WHITE_PAIR, aboutInfo);
}
