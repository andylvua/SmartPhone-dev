//
// Created by paul on 1/14/23.
//

#include <QProcess>
#include <utility>
#include "../../Inc/cli/cli.h"
#include "../../Inc/logging.h"
#include "../../Inc/cli/color_print.h"


const auto cli_logger = spdlog::basic_logger_mt("cli", "../logs/log.txt", true);
RotaryDial rtx;

CLI::CLI(Modem &modem) : modem(modem) {
    prepareScreens();
    rtx.setup();

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

    for (int i = 1; i < number.length(); ++i) {
        if (!isdigit(number[i])) {
            printColored(RED, "Invalid number");
            return false;
        }
    }

    return true;
}

void CLI::renderScreen() const {
    system("clear");

    printColored(BOLDWHITE, currentScreen->screenName.toStdString());

    for (const auto& notification: currentScreen->notifications) {
        printColored(WHITE, notification.toStdString());
    }

    for (auto const &option: currentScreen->screenOptions) {
        if (option.contains("enter")) {
            std::string optionString = option.split("enter")[0].toStdString();
            printColored(WHITE, optionString, false);
            printColored(GREEN, "enter", false);
            printColored(WHITE, ")");
        } else {
            printColored(WHITE, option.toStdString());
        }
    }
}

void CLI::changeScreen(std::shared_ptr<Screen> screen) {
    currentScreen = std::move(screen);
}

void CLI::changeScreen(const QString &screenName) {
    for (const auto& screen: screens) {
        if (screen->screenName == screenName) {
            currentScreen = screen;
        }
    }
}

void CLI::handleIncomingCall(const QString& number) {
    for (const auto& screen: screens) {
        if (screen->screenName == "Incoming Call" || screen->screenName == "In Call") {
            screen->addNotification("Incoming call from +" + number);
        }
    }
    changeScreen("Incoming Call");
    renderScreen();
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
    renderScreen();
}

void CLI::mainScreenHandler(const char *line) {
    if (strcmp(line, "0") == 0) {
        exit(0);
    }
    if (strcmp(line, "1") == 0) {
        changeScreen("Phone");
        renderScreen();
    }
    if (strcmp(line, "2") == 0) {
        changeScreen("SMS");
        renderScreen();
    }
    if (strcmp(line, "3") == 0) {
        changeScreen("USSD Console");
        renderScreen();
    }
    if (strcmp(line, "4") == 0) {
        changeScreen("AT Console");
        renderScreen();
    }
    if (strcmp(line, "5") == 0) {
        changeScreen("Logs");
        renderScreen();
    }
}

void CLI::incomingCallScreenHandler(const char *line) {
    if (strcmp(line, "1") == 0 || line[0] == '\0') {
        printColored(YELLOW, "Answering call");
        modem.answer();
        changeScreen("In Call");
        renderScreen();

    }
    if (strcmp(line, "0") == 0) {
        printColored(YELLOW, "Rejecting call");
        modem.hangUp();
        for (const auto& screen: screens) {
            if (screen->screenName == "Incoming Call" || screen->screenName == "In Call") {
                screen->notifications.clear();
            }
        }
        changeScreen("Main");
        renderScreen();
    }
}

void CLI::phoneScreenHandler(const char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0) {

        changeScreen("Call");
        renderScreen();
    }
    if (strcmp(line, "2") == 0) {
        changeScreen("Contacts");
        renderScreen();
    }
    if (strcmp(line, "3") == 0) {
        printColored(YELLOW, "Listing call history");
        Modem::listCalls();
    }
}

void CLI::smsScreenHandler(const char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0) {
        changeScreen("Send SMS");
        renderScreen();
    }
    if (strcmp(line, "2") == 0) {
        for (const auto& screen: screens) {
            if (screen->screenName == "Main") {
                screen->removeNotification("    *New SMS");
            }
        }
        printColored(YELLOW, "Listing messages");
        Modem::listMessages();
    }
}

void CLI::ussdScreenHandler(const char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0) {
        printColored(BOLDYELLOW, "In development(Coming soon)");
    }
}

void CLI::atScreenHandler(const char *line) {
    if (strcmp(line, "0") == 0) {
        modem.disableConsoleMode();
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0 || line[0] == '\0') {
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
    }
}

void CLI::logsScreenHandler(const char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0 || line[0] == '\0') {
        printColored(GREEN, "Opening logs");
        system("xdg-open ./../logs/log.txt");
    }
}

void CLI::callScreenHandler(const char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0 || line[0] == '\0') {
        std::string number;
        printColored(YELLOW, "Enter number");
        printColored(YELLOW, "Read from rotary dial or keyboard? (r/k)");
        char input;
        std::cin >> input;

        if (input == 'r') {
            printColored(YELLOW, "Reading from rotary dial");
            number = rtx.listen_for_number(modem.outStream);
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
        if (modem.call(number)) {
            for (const auto &screen: screens) {
                if (screen->screenName == "In Call") {
                    screen->addNotification("Calling " + QString::fromStdString(number));
                }
            }
            changeScreen("In Call");
        }
        else{
            printColored(RED, "Call failed");
        }

        renderScreen();
    }
}

void CLI::inCallScreenHandler(const char *line) {
    if (strcmp(line, "0") == 0 || line[0] == '\0') {
        modem.hangUp();
        for (const auto& screen: screens) {
            if (screen->screenName == "Incoming Call" || screen->screenName == "In Call") {
                screen->notifications.clear();
            }
        }

        printColored(YELLOW, "Hanging up");
        changeScreen("Main");
        renderScreen();
    }
}

void CLI::contactsScreenHandler(const char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    } else if (strcmp(line, "1") == 0) {
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
            number = rtx.listen_for_number(modem.outStream);
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
        renderScreen();
    } else if (strcmp(line, "2") == 0) {
        printColored(YELLOW, "Deleting contact");
        std::string name;
        printColored(YELLOW, "Enter name");
        std::cin >> name;

        Modem::removeContact(name);
        changeScreen("Contacts");
        renderScreen();
    } else if (strcmp(line, "3") == 0) {
        printColored(YELLOW, "Listing contacts");
        Modem::listContacts();
        changeScreen("Contacts");
    }
}

void CLI::sendSMSScreenHandler(const char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    } else if (strcmp(line, "1") == 0 || line[0] == '\0') {
        std::string number;
        std::string message;
        printColored(YELLOW, "Enter number: ");
        printColored(YELLOW, "Read from rotary dial or keyboard? (r/k)");
        char input;
        std::cin >> input;

        if (input == 'r') {
            printColored(YELLOW, "Reading from rotary dial");
            number = rtx.listen_for_number(modem.outStream);
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
    }
}

void CLI::listen() {
    cli_logger->flush_on(spdlog::level::debug);
    SPDLOG_LOGGER_INFO(cli_logger, "CLI listener started.");

    const char *line;
    renderScreen();
    printColored(BOLDWHITE, ">>> ", false);

    // if escape is pressed, print to QDebug about it
    while ((line = readline(""))) {
        if (strcmp(line, "exit") == 0) {
            exit(0);
        }

        if (currentScreen->screenName == "Main") {
            mainScreenHandler(line);
        } else if (currentScreen->screenName == "Incoming Call") {
            incomingCallScreenHandler(line);
        } else if (currentScreen->screenName == "Phone") {
            phoneScreenHandler(line);
        } else if (currentScreen->screenName == "Call") {
            callScreenHandler(line);
        } else if (currentScreen->screenName == "In Call") {
            inCallScreenHandler(line);
        } else if (currentScreen->screenName == "Contacts") {
            contactsScreenHandler(line);
        } else if (currentScreen->screenName == "SMS") {
            smsScreenHandler(line);
        } else if (currentScreen->screenName == "Send SMS") {
            sendSMSScreenHandler(line);
        } else if (currentScreen->screenName == "USSD Console") {
            ussdScreenHandler(line);
        } else if (currentScreen->screenName == "AT Console") {
            atScreenHandler(line);
        } else if (currentScreen->screenName == "Logs") {
            logsScreenHandler(line);
        } else {
            printColored(RED, "Unknown screen");
        }

        printColored(BOLDWHITE, ">>> ", false);
    }
}

void CLI::prepareScreens() {
    auto mainScreen = std::make_shared<Screen>("Main", nullptr);
    mainScreen->addScreenOption("0. Exit");
    mainScreen->addScreenOption("1. Phone");
    mainScreen->addScreenOption("2. SMS");
    mainScreen->addScreenOption("3. USSD Console");
    mainScreen->addScreenOption("4. AT Console");
    mainScreen->addScreenOption("5. Logs");

    auto incomingCallScreen = std::make_shared<Screen>("Incoming Call", mainScreen);
    incomingCallScreen->addScreenOption("0. Hang up");
    incomingCallScreen->addScreenOption("1. Answer (enter)");

    auto phoneScreen = std::make_shared<Screen>("Phone", mainScreen);
    phoneScreen->addScreenOption("0. Back");
    phoneScreen->addScreenOption("1. Call");
    phoneScreen->addScreenOption("2. Contacts");
    phoneScreen->addScreenOption("3. Call history");

    auto callScreen = std::make_shared<Screen>("Call", phoneScreen);
    callScreen->addScreenOption("0. Return");
    callScreen->addScreenOption("1. Make Call (enter)");

    auto inCallScreen = std::make_shared<Screen>("In Call", callScreen);
    inCallScreen->addScreenOption("0. Hang up (enter)");

    auto contactsScreen = std::make_shared<Screen>("Contacts", phoneScreen);
    contactsScreen->addScreenOption("0. Back");
    contactsScreen->addScreenOption("1. Add Contact");
    contactsScreen->addScreenOption("2. Remove Contact");
    contactsScreen->addScreenOption("3. View Contacts");

    auto smsScreen = std::make_shared<Screen>("SMS", mainScreen);
    smsScreen->addScreenOption("0. Back");
    smsScreen->addScreenOption("1. Send SMS");
    smsScreen->addScreenOption("2. Messages");

    auto sendSMSScreen = std::make_shared<Screen>("Send SMS", smsScreen);
    sendSMSScreen->addScreenOption("0. Back");
    sendSMSScreen->addScreenOption("1. Write SMS (enter)");

    auto logScreen = std::make_shared<Screen>("Logs", mainScreen);
    logScreen->addScreenOption("0. Back");
    logScreen->addScreenOption("1. View Logs (enter)");

    auto ussdScreen = std::make_shared<Screen>("USSD Console", mainScreen);
    ussdScreen->addScreenOption("0. Back");
    ussdScreen->addScreenOption("1. Send USSD (enter)");

    auto atScreen = std::make_shared<Screen>("AT Console", mainScreen);
    atScreen->addScreenOption("0. Back");
    atScreen->addScreenOption("1. Send AT Command (enter)");

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

    currentScreen = screens[0];
}
