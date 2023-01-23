//
// Created by paul on 1/14/23.
//

#include <QProcess>
#include <utility>
#include "../../Inc/cli/cli.h"
#include "../../Inc/logging.h"
#include <string>

#define GO_BACK [&](){gotoParentScreen();}

const auto cli_logger = spdlog::basic_logger_mt("cli", "../logs/log.txt", true);
//RotaryDial rtx;

CLI::CLI(Modem &modem) : modem(modem) {
    prepareScreens();
//    rtx.setup();

    connect(&modem, SIGNAL(incomingCall(QString)),
            this, SLOT(handleIncomingCall(QString)));
    connect(&modem, SIGNAL(incomingSMS()), this, SLOT(handleIncomingSMS()));
    connect(&modem, SIGNAL(callEnded()), this, SLOT(handleCallEnded()));
}

bool checkNumber(std::string &number) {
    if (number[0] != '+') {
        number = '+' + number;
    }

    if (number.length() != 13) {
        printColored(RED, "Invalid number");
        return false;
    }

    for (size_t i = 1; i < number.length(); ++i) {
        if (!isdigit(number[i])) {
            printColored(RED, "Invalid number");
            return false;
        }
    }

    return true;
}

void CLI::renderScreen() const {
    int activeOptionIndex = currentScreen->getActiveOption();
    clear();

    printColored(WHITE, currentScreen->screenName.toStdString(), true, true);

    for (const auto& notification: currentScreen->notifications) {
        printColored(WHITE, notification.toStdString());
    }

    for (size_t i = 0; i < currentScreen->screenOptions.size(); ++i) {
        QString option = currentScreen->screenOptions[i].optionName;
        printColored(WHITE, std::to_string(i) + ". ", false);

        int color = (i == static_cast<size_t>(activeOptionIndex)) ? FILLED_WHITE : WHITE;
        printColored(color, option.toStdString());
    }
}

void CLI::changeScreen(const QString &screenName) {
    currentScreen = screenMap[screenName];
    renderScreen();
}

void CLI::gotoParentScreen() {
    if (currentScreen->parentScreen != nullptr) {
        currentScreen = currentScreen->parentScreen;
        renderScreen();
    }
}

void CLI::handleIncomingCall(const QString& number) {
    for (const auto& screen: screens) {
        if (screen->screenName == "Incoming Call" || screen->screenName == "In Call") {
            screen->addNotification("Incoming call from +" + number);
        }
    }
    changeScreen("Incoming Call");
}

void CLI::handleIncomingSMS() {
    qDebug() << "Incoming SMS";

    for (const auto& screen: screens) {
        if (screen->screenName == "Main" && screen->notifications.empty()) {
            screen->addNotification("    *New SMS");
        }
    }
    renderScreen();
}

void CLI::handleCallEnded() {
    for (const auto& screen: screens) {
        if (screen->screenName == "In Call" || screen->screenName == "Incoming Call") {
            screen->notifications.clear();
        }
    }

    changeScreen("Main");
}

void CLI::incrementActiveOption() const {
    currentScreen->activeOption++;
}

void CLI::decrementActiveOption() const {
    if (currentScreen->activeOption > -1) {
        currentScreen->activeOption--;
    }
}

void CLI::listen() const {
    cli_logger->flush_on(spdlog::level::debug);
    SPDLOG_LOGGER_INFO(cli_logger, "CLI listener started.");

    Screen::initScreen();
    renderScreen();

    int ch;

    while (true) {
        if ((ch = getch()) == ESC) {
            Screen::releaseScreen();
            exit(0);
        }

        switch(ch) {
            case KEY_UP:
                CLI::decrementActiveOption();
                renderScreen();
                break;
            case KEY_DOWN:
                CLI::incrementActiveOption();
                renderScreen();
                break;
            case '\n':
                if (currentScreen->activeOption == -1) {
                    break;
                }

                currentScreen->screenOptions[currentScreen->getActiveOption()].execute();
                break;

            default: break;
        }
    }
}

void CLI::prepareScreens() {
    auto mainScreen = ScreenSharedPtr("Main", nullptr);
    auto incomingCallScreen = ScreenSharedPtr("Incoming Call", mainScreen);
    auto phoneScreen = ScreenSharedPtr("Phone", mainScreen);
    auto callScreen = ScreenSharedPtr("Call", phoneScreen);
    auto inCallScreen = ScreenSharedPtr("In Call", callScreen);
    auto contactsScreen = ScreenSharedPtr("Contacts", phoneScreen);
    auto smsScreen = ScreenSharedPtr("SMS", mainScreen);
    auto sendSMSScreen = ScreenSharedPtr("Send SMS", smsScreen);
    auto logScreen = ScreenSharedPtr("Logs", mainScreen);
    auto ussdScreen = ScreenSharedPtr("USSD Console", mainScreen);
    auto atScreen = ScreenSharedPtr("AT Console", mainScreen);

    mainScreen->addScreenOption("Exit", []() {
        exit(0);
    });
    mainScreen->addScreenOption("Phone", [this]() {
        changeScreen("Phone");
    });
    mainScreen->addScreenOption("SMS", [this]() {
        changeScreen("SMS");
    });
    mainScreen->addScreenOption("USSD Console", [this]() {
        changeScreen("USSD Console");
    });
    mainScreen->addScreenOption("AT Console", [this]() {
        changeScreen("AT Console");
    });
    mainScreen->addScreenOption("Logs", [this]() {
        changeScreen("Logs");
    });

    incomingCallScreen->addScreenOption("Hang up", [this]() {
        printColored(YELLOW, "Rejecting call");
        modem.hangUp();
        for (const auto& screen: screens) {
            if (screen->screenName == "Incoming Call" || screen->screenName == "In Call") {
                screen->notifications.clear();
            }
        }
        changeScreen("Main");
    });
    incomingCallScreen->addScreenOption("Answer", [this]() {
        printColored(YELLOW, "Answering call");
        modem.answer();

        changeScreen("In Call");
    });

    phoneScreen->addScreenOption("Back", GO_BACK);
    phoneScreen->addScreenOption("Call", [this]() {
        changeScreen("Call");
    });
    phoneScreen->addScreenOption("Contacts", [this]() {
        changeScreen("Contacts");
    });
    phoneScreen->addScreenOption("Call history", []() {
        printColored(YELLOW, "Call history:");
        Modem::listCalls();
    });

    callScreen->addScreenOption("Return", GO_BACK);

    callScreen->addScreenOption("Call", [this]() {
        std::string number;
        printColored(YELLOW, "Enter number");
        printColored(YELLOW, "Read from rotary dial or keyboard? (r/k)");
        char input;
        std::cin >> input;

        if (input == 'r') {
            printColored(YELLOW, "Reading from rotary dial");
//            number = rtx.listen_for_number(modem.outStream);
        } else if (input == 'k') {
            printColored(YELLOW, "Reading from keyboard");
            std::cin >> number;
        } else {
            printColored(RED, "Invalid input");
            return;
        }

        if (!checkNumber(number)) {
            return;
        }

        printColored(YELLOW, "Calling...");
        modem.call(number);

        for (const auto& screen: screens) {
            if (screen->screenName == "In Call") {
                screen->addNotification("Calling " + QString::fromStdString(number));
            }
        }

        changeScreen("In Call");
    });

    inCallScreen->addScreenOption("Hang up", [this]() {
        modem.hangUp();
        for (const auto& screen: screens) {
            if (screen->screenName == "Incoming Call" || screen->screenName == "In Call") {
                screen->notifications.clear();
            }
        }

        printColored(YELLOW, "Hanging up");
        changeScreen("Main");
    });

    contactsScreen->addScreenOption("Back", GO_BACK);
    contactsScreen->addScreenOption("Add Contact", [this]() {
        printColored(YELLOW, "Adding contact");
        std::string name;
        std::string number;
        printColored(YELLOW, "Enter name");
        std::cin >> name;
        printColored(YELLOW, "Enter number");
        printColored(YELLOW, "Read from rotary dial or keyboard? (r/k)");
        char input;
        std::cin >> input;

        if (input == 'r') {
            printColored(YELLOW, "Reading from rotary dial");
//            number = rtx.listen_for_number(modem.outStream);
        } else if (input == 'k') {
            printColored(YELLOW, "Reading from keyboard");
            std::cin >> number;
        } else {
            printColored(RED, "Invalid input");
            return;
        }

        if (!checkNumber(number)) {
            return;
        }

        Modem::addContact(name, number);
        changeScreen("Contacts");
    });
    contactsScreen->addScreenOption("Remove Contact", [this]() {
        printColored(YELLOW, "Deleting contact");
        std::string name;
        printColored(YELLOW, "Enter name");
        std::cin >> name;

        Modem::removeContact(name);
        changeScreen("Contacts");
    });
    contactsScreen->addScreenOption("View Contacts", []() {
        printColored(YELLOW, "Listing contacts");
        Modem::listContacts();
    });

    smsScreen->addScreenOption("Back", GO_BACK);
    smsScreen->addScreenOption("Messages", [this]() {
        for (const auto& screen: screens) {
            if (screen->screenName == "Main") {
                screen->removeNotification("    *New SMS");
            }
        }
        printColored(YELLOW, "Listing messages");
        Modem::listMessages();
    });
    smsScreen->addScreenOption("Send SMS", [this]() {
        changeScreen("Send SMS");
    });

    sendSMSScreen->addScreenOption("Back", GO_BACK);
    sendSMSScreen->addScreenOption("Write SMS", [this]() {
        std::string number;
        std::string message;
        printColored(YELLOW, "Enter number: ");
        printColored(YELLOW, "Read from rotary dial or keyboard? (r/k)");
        char input;
        std::cin >> input;

        if (input == 'r') {
            printColored(YELLOW, "Reading from rotary dial");
//            number = rtx.listen_for_number(modem.outStream);
        } else if (input == 'k') {
            printColored(YELLOW, "Reading from keyboard");
            std::cin >> number;
        } else {
            printColored(RED, "Invalid input");
            return;
        }

        if (!checkNumber(number)) {
            return;
        }

        printColored(YELLOW, "Enter message: ");
        std::getline(std::cin >> std::ws, message);
        printColored(YELLOW, "Sending SMS");

        modem.message(number, message);
    });

    logScreen->addScreenOption("Back", GO_BACK);
    logScreen->addScreenOption("View Logs", []() {
        printColored(GREEN, "Opening logs");
        system("xdg-open ./../logs/log.txt");
    });

    ussdScreen->addScreenOption("Back", GO_BACK);
    ussdScreen->addScreenOption("Send USSD Command", []() {
        printColored(YELLOW, "In development...\n");
    });

    atScreen->addScreenOption("Back", [this]() {
        modem.disableConsoleMode();
        gotoParentScreen();
    });
    atScreen->addScreenOption("Send AT Command", [this]() {
        modem.enableConsoleMode();
        printColored(GREEN, "AT Console mode enabled. To exit, type 'exit'");
        std::string at{};

        printColored(WHITE, "Enter commands:");
        while (true) {
            std::cin >> at;
            if (at == "exit") {
                break;
            }
            modem.sendConsoleCommand(QString::fromStdString(at));
        }

        modem.disableConsoleMode();
        renderScreen();
    });

    screens.push_back(mainScreen);
    screens.push_back(incomingCallScreen);
    screens.push_back(phoneScreen);
    screens.push_back(callScreen);
    screens.push_back(inCallScreen);
    screens.push_back(contactsScreen);
    screens.push_back(smsScreen);
    screens.push_back(sendSMSScreen);
    screens.push_back(logScreen);
    screens.push_back(ussdScreen);
    screens.push_back(atScreen);

    for (const auto& screen : screens) {
        CLI::screenMap[screen->screenName] = screen;
    }

    currentScreen = screens[0];
}
