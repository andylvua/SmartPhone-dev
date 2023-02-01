//
// Created by Andrew Yaroshevych on 01.02.2023.
//

#include "../../../Inc/cli/cli.hpp"
#include "../../../Inc/cli/utils/cli_utils.hpp"
#include "../../../Inc/cli/utils/ncurses_utils.hpp"
#include "../../../Inc/cli/defenitions/colors.hpp"
#include "../../../Inc/modem/utils/cache_manager.hpp"

void render(const std::shared_ptr<Screen> &screen) {
    int activeOptionIndex = screen->getActiveOption();

    printColored(WHITE_PAIR, screen->screenName.toStdString(), true, true);

    for (const auto &notification: screen->notifications) {
        printColored(WHITE_PAIR, notification.toStdString());
    }

    for (int i = 0; i < static_cast<int>(screen->screenOptions.size()); ++i) {
        auto option = screen->screenOptions[i];

        int color = (i == (activeOptionIndex)) ? FILLED_WHITE_PAIR : WHITE_PAIR;

        if (i == 0 && i == activeOptionIndex) {
            color = FILLED_RED_PAIR;
        }
        if (option->isSwitcher) {
            if (i == activeOptionIndex){
                color = option->switcher ? FILLED_GREEN_PAIR : FILLED_RED_PAIR;
            } else {
                color = option->switcher ? GREEN_PAIR : RED_PAIR;
            }
        }

        if (!option->isAvailable) {
            color = (i == (activeOptionIndex)) ? FILLED_RED_PAIR : RED_PAIR;
        }

        printColored(color, option->optionName.toStdString());
    }

    refresh();
}

bool checkNumber(std::string &number) {
    if (number[0] != '+') {
        number = '+' + number;
    }

    if (number.length() != 13) {
        printColored(RED_PAIR, "Invalid number: " + number);
        return false;
    }

    for (size_t i = 1; i < number.length(); ++i) {
        if (!isdigit(number[i])) {
            printColored(RED_PAIR, "Invalid number");
            return false;
        }
    }

    return true;
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
    std::string number;
    printColored(YELLOW_PAIR, "Enter number");
#ifdef BUILD_ON_RASPBERRY
    printColored(YELLOW_PAIR, "Read from rotary dial or keyboard? (r/k)");
    std::string input;
    input = readString();

    if (input == "r") {
        printColored(YELLOW_PAIR, "Reading from rotary dial");
        number = rtx.listen_for_number(modem.outStream);
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

    CLI::screenMap["In Call"]->addNotification("Calling " + QString::fromStdString(number));
    changeScreen("In Call");
}

void CLI::call(const QString &number) {
    printColored(YELLOW_PAIR, "Calling...");
    modem.call(number.toStdString());

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
    std::string name;
    std::string number;
    printColored(YELLOW_PAIR, "Enter name");
    name = readString();
    printColored(YELLOW_PAIR, "Enter number");
#ifdef BUILD_ON_RASPBERRY
    printColored(YELLOW_PAIR, "Read from rotary dial or keyboard? (r/k)");
    std::string input;
    input = readString();

    if (input == "r") {
        printColored(YELLOW_PAIR, "Reading from rotary dial");
        number = rtx.listen_for_number(modem.outStream);
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
    std::vector<Contact> contacts = CacheManager::getContacts();
    auto contactsPage = CLI::screenMap["Contacts Page"];

    contactsPage->screenOptions.erase(
            contactsPage->screenOptions.begin() + 1,
            contactsPage->screenOptions.end());

    for (const auto &contact: contacts) {
        contactsPage->addScreenOption(
                contact.name + ": " + contact.number,
                [contact, this]() {
                    auto contactScreen = std::make_shared<ContactScreen>(
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
    std::string number;
    std::string message;
    printColored(YELLOW_PAIR, "Enter number: ");
#ifdef BUILD_ON_RASPBERRY
    printColored(YELLOW_PAIR, "Read from rotary dial or keyboard? (r/k)");
    std::string input;
    input = readString();

    if (input == "r") {
        printColored(YELLOW_PAIR, "Reading from rotary dial");
        number = rtx.listen_for_number(modem.outStream);
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
    std::string message;
    printColored(YELLOW_PAIR, "Enter message: ");
    message = readString();
    printColored(YELLOW_PAIR, "Sending SMS");

    modem.message(number.toStdString(), message);
}

void CLI::viewLogs() {
    printColored(GREEN_PAIR, "Opening logs file");
    CacheManager::listLogs();
}

void CLI::setMessageMode() {
    for (const auto& option: CLI::screenMap["Debug Settings"]->screenOptions) {
        if (option->optionName == "Message Mode" && option->isSwitcher) {
            bool success = modem.setMessageMode(!option->getState());
            if (success) {
                option->switchState();
            } else {
                printColored(RED_PAIR, "Failed to set Message Mode");
            }
        }
    }
    updateScreen();
}

void CLI::setNumberID() {
    for (const auto& option: CLI::screenMap["Debug Settings"]->screenOptions) {
        if (option->optionName == "Number Identifier" && option->isSwitcher) {
            bool success = modem.setNumberID(!option->getState());
            if (success) {
                option->switchState();
            } else {
                printColored(RED_PAIR, "Failed to set Number Identifier");
            }
        }
    }
    updateScreen();
}

void CLI::setEchoMode(){
    for (const auto& option: CLI::screenMap["Debug Settings"]->screenOptions) {
        if (option->optionName == "Echo Mode" && option->isSwitcher){
            bool success = modem.setEchoMode(!option->getState());
            if (success) {
                option->switchState();
            } else {
                printColored(RED_PAIR, "Failed to set Echo Mode");
            }
        }
    }
    updateScreen();
}

void CLI::aboutDevice(){
    changeScreen("About Device");
    QString aboutInfo = modem.aboutDevice();
    printColored(WHITE_PAIR, aboutInfo.toStdString());
}
