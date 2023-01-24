//
// Created by paul on 1/14/23.
//

#include <QProcess>
#include <utility>
#include "../../Inc/cli/cli.h"
#include "../../Inc/logging.h"
#include <string>

const auto cli_logger = spdlog::basic_logger_mt("cli", "../logs/log.txt", true);

#ifdef BUILD_ON_RASPBERRY
RotaryDial rtx;
#endif

CLI::CLI(Modem &modem) : modem(modem) {
    prepareScreens();
#ifdef BUILD_ON_RASPBERRY
    rtx.setup();
#endif

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
        printColored(RED, "Invalid number: " + number);
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

void _render(std::shared_ptr<Screen> screen) {
    int activeOptionIndex = screen->getActiveOption();

    printColored(WHITE, screen->screenName.toStdString(), true, true);

    for (const auto& notification: screen->notifications) {
        printColored(WHITE, notification.toStdString());
    }

    for (size_t i = 0; i < screen->screenOptions.size(); ++i) {
        QString option = screen->screenOptions[i].optionName;
        printColored(WHITE, std::to_string(i) + ". ", false);

        int color = (i == static_cast<size_t>(activeOptionIndex)) ? FILLED_WHITE : WHITE;
        printColored(color, option.toStdString());
    }
}

void CLI::renderScreen() {
    clear();
    _render(currentScreen);

    lastRenderedScreen = currentScreen;
}

void CLI::updateScreen() {
    if (lastRenderedScreen == nullptr) {
        lastRenderedScreen = currentScreen;
        renderScreen();
        return;
    }

    move(0, 0);
    _render(currentScreen);
    lastRenderedScreen = currentScreen;
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
    CLI::screenMap["Incoming Call"]->addNotification("Incoming call from +" + number);
    CLI::screenMap["In Call"]->addNotification("Incoming call from +" + number);
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

void CLI::incrementActiveOption() {
    currentScreen->activeOption++;
    updateScreen();
}

void CLI::decrementActiveOption() {
    if (currentScreen->activeOption > -1) {
        currentScreen->activeOption--;
    }
    updateScreen();
}

void CLI::listen() {
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
                break;
            case KEY_DOWN:
                CLI::incrementActiveOption();
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

void CLI::_rejectCall() {
    printColored(YELLOW, "Rejecting call");
    modem.hangUp();
    CLI::screenMap["Incoming Call"]->notifications.clear();
    CLI::screenMap["In Call"]->notifications.clear();
    changeScreen("Main");
}

void CLI::_answerCall() {
    printColored(YELLOW, "Answering call");
    modem.answer();

    changeScreen("In Call");
}

void CLI::_viewCallHistory() {
    printColored(YELLOW, "Call history:");
    Modem::listCalls();
    renderScreen();
}

void CLI::_call() {
    std::string number;
    printColored(YELLOW, "Enter number");
#ifdef BUILD_ON_RASPBERRY
    printColored(YELLOW, "Read from rotary dial or keyboard? (r/k)");
    char input;
    input = readString();

    if (input == 'r') {
        printColored(YELLOW, "Reading from rotary dial");
        number = rtx.listen_for_number(modem.outStream);
    } else if (input == 'k') {
        printColored(YELLOW, "Reading from keyboard");
        number = readString();
    } else {
        printColored(RED, "Invalid input");
        return;
    }
#else
    number = readString();
#endif

    if (!checkNumber(number)) {
        return;
    }

    printColored(YELLOW, "Calling...");
    modem.call(number);

    CLI::screenMap["In Call"]->addNotification("Calling " + QString::fromStdString(number));

    changeScreen("In Call");
}

void CLI::_hangUp() {
    modem.hangUp();
    CLI::screenMap["In Call"]->notifications.clear();

    printColored(YELLOW, "Hanging up");
    changeScreen("Main");
}

void CLI::_addContact() {
    printColored(YELLOW, "Adding contact");
    std::string name;
    std::string number;
    printColored(YELLOW, "Enter name");
    name = readString();
    printColored(YELLOW, "Enter number");
#ifdef BUILD_ON_RASPBERRY
    printColored(YELLOW, "Read from rotary dial or keyboard? (r/k)");
    char input;
    input = readString();

    if (input == 'r') {
        printColored(YELLOW, "Reading from rotary dial");
        number = rtx.listen_for_number(modem.outStream);
    } else if (input == 'k') {
        printColored(YELLOW, "Reading from keyboard");
        number = readString();
    } else {
        printColored(RED, "Invalid input");
        return;
    }
#else
    number = readString();
#endif

    if (!checkNumber(number)) {
        return;
    }

    Modem::addContact(name, number);
    changeScreen("Contacts");
}

void CLI::_deleteContact() {
    printColored(YELLOW, "Deleting contact");
    std::string name;
    printColored(YELLOW, "Enter name");
    name = readString();

    Modem::removeContact(name);
    changeScreen("Contacts");
}

void CLI::_viewContacts() {
    printColored(YELLOW, "Listing contacts");
    Modem::listContacts();
    renderScreen();
}

void CLI::_viewMessages() {
    CLI::screenMap["Main"]->notifications.clear();
    printColored(YELLOW, "Listing messages");
    Modem::listMessages();
    renderScreen();
}

void CLI::_sendMessage() {
    std::string number;
    std::string message;
    printColored(YELLOW, "Enter number: ");
#ifdef BUILD_ON_RASPBERRY
    printColored(YELLOW, "Read from rotary dial or keyboard? (r/k)");
    char input;
    input = readString();

    if (input == 'r') {
        printColored(YELLOW, "Reading from rotary dial");
        number = rtx.listen_for_number(modem.outStream);
    } else if (input == 'k') {
        printColored(YELLOW, "Reading from keyboard");
        number = readString();
    } else {
        printColored(RED, "Invalid input");
        return;
    }
#else
    number = readString();
#endif

    if (!checkNumber(number)) {
        return;
    }

    printColored(YELLOW, "Enter message: ");
    message = readString();
    printColored(YELLOW, "Sending SMS");

    modem.message(number, message);
}

void CLI::_viewLogs() {
    printColored(GREEN, "Opening logs");
    system("xdg-open ./../logs/log.txt");
}

void CLI::_sendUSSD() {
    printColored(YELLOW, "In development...\n");
}

void CLI::_sendATCommand() {
    modem.enableConsoleMode();
    printColored(GREEN, "AT Console mode enabled. To exit, type 'exit'");
    std::string at{};

    printColored(WHITE, "Enter commands:");
    while (true) {
        at = readString();
        if (at == "exit") {
            break;
        }
        modem.sendConsoleCommand(QString::fromStdString(at));
    }

    modem.disableConsoleMode();
    renderScreen();
}

void CLI::_disableATConsole() {
    modem.disableConsoleMode();
    gotoParentScreen();
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
    mainScreen->addScreenOption("Phone", CHANGE_SCREEN("Phone"));
    mainScreen->addScreenOption("SMS", CHANGE_SCREEN("SMS"));
    mainScreen->addScreenOption("USSD Console", CHANGE_SCREEN("USSD Console"));
    mainScreen->addScreenOption("AT Console", CHANGE_SCREEN("AT Console"));
    mainScreen->addScreenOption("Logs", CHANGE_SCREEN("Logs"));

    incomingCallScreen->addScreenOption("Reject call", EXECUTE_METHOD(_rejectCall));

    incomingCallScreen->addScreenOption("Answer", EXECUTE_METHOD(_answerCall));

    phoneScreen->addScreenOption("Back", GO_BACK);
    phoneScreen->addScreenOption("Call", CHANGE_SCREEN("Call"));
    phoneScreen->addScreenOption("Contacts", CHANGE_SCREEN("Contacts"));
    phoneScreen->addScreenOption("Call history", EXECUTE_METHOD(_viewCallHistory));

    callScreen->addScreenOption("Return", GO_BACK);

    callScreen->addScreenOption("Call", EXECUTE_METHOD(_call));

    inCallScreen->addScreenOption("Hang up", EXECUTE_METHOD(_hangUp));

    contactsScreen->addScreenOption("Back", GO_BACK);
    contactsScreen->addScreenOption("Add Contact", EXECUTE_METHOD(_addContact));
    contactsScreen->addScreenOption("Delete Contact", EXECUTE_METHOD(_deleteContact));

    contactsScreen->addScreenOption("View Contacts", EXECUTE_METHOD(_viewContacts));

    smsScreen->addScreenOption("Back", GO_BACK);
    smsScreen->addScreenOption("Messages", EXECUTE_METHOD(_viewMessages));
    smsScreen->addScreenOption("Send SMS", CHANGE_SCREEN("Send SMS"));
    sendSMSScreen->addScreenOption("Back", GO_BACK);
    sendSMSScreen->addScreenOption("Write SMS", EXECUTE_METHOD(_sendMessage));

    logScreen->addScreenOption("Back", GO_BACK);
    logScreen->addScreenOption("View Logs", EXECUTE_METHOD(_viewLogs));

    ussdScreen->addScreenOption("Back", GO_BACK);
    ussdScreen->addScreenOption("Send USSD Command", EXECUTE_METHOD(_sendUSSD));

    atScreen->addScreenOption("Back", EXECUTE_METHOD(_disableATConsole));
    atScreen->addScreenOption("Send AT Command", EXECUTE_METHOD(_sendATCommand));


    CLI::screenMap = {
            {"Main", mainScreen},
            {"Incoming Call", incomingCallScreen},
            {"Phone", phoneScreen},
            {"Call", callScreen},
            {"In Call", inCallScreen},
            {"Contacts", contactsScreen},
            {"SMS", smsScreen},
            {"Send SMS", sendSMSScreen},
            {"Logs", logScreen},
            {"USSD Console", ussdScreen},
            {"AT Console", atScreen}
    };

    currentScreen = mainScreen;
}
