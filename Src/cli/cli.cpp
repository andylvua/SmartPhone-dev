//
// Created by paul on 1/14/23.
//

#include "../../Inc/logging.hpp"
#include "../../Inc/cli/cli.hpp"
#include "../../Inc/cli/cli_macros.hpp"
#include "../../Inc/cli/ncurses_utils.hpp"
#include "../../Inc/modem/cache_manager.hpp"
#include "../../Inc/cli/colors.hpp"
#include <QProcess>
#include <string>

const auto cliLogger = spdlog::basic_logger_mt("cli", "../logs/log.txt", true);

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

void render(const std::shared_ptr<Screen>& screen) {
    int activeOptionIndex = screen->getActiveOption();

    printColored(WHITE_PAIR, screen->screenName.toStdString(), true, true);

    for (const auto& notification: screen->notifications) {
        printColored(WHITE_PAIR, notification.toStdString());
    }

    for (size_t i = 0; i < screen->screenOptions.size(); ++i) {
        QString option = screen->screenOptions[i].optionName;
        int color = (i == static_cast<size_t>(activeOptionIndex)) ? FILLED_WHITE_PAIR : WHITE_PAIR;
        if (i == 0 && i == static_cast<size_t>(activeOptionIndex)) {
            color = FILLED_RED_PAIR;
        }
        printColored(color, option.toStdString());
    }

    refresh();
}

void CLI::renderScreen() const {
    clear();
    render(currentScreen);
}

void CLI::updateScreen() const {
    move(0, 0);
    render(currentScreen);
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

void CLI::incrementActiveOption() const {
    currentScreen->activeOption++;
    updateScreen();
}

void CLI::decrementActiveOption() const {
    if (currentScreen->activeOption > -1) {
        currentScreen->activeOption--;
    }
    updateScreen();
}

void CLI::listen() const {
    cliLogger->flush_on(spdlog::level::debug);
    SPDLOG_LOGGER_INFO(cliLogger, "CLI listener started.");

    initScreen();
    renderScreen();

    int ch;

    while (true) {
        if ((ch = getch()) == 27) { // ESC
            releaseScreen();
            exit(0);
        }

        switch(ch) {
            case KEY_UP: CLI::decrementActiveOption();
                break;
            case KEY_DOWN: CLI::incrementActiveOption();
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

void CLI::deleteContact() {
    printColored(YELLOW_PAIR, "Deleting contact");
    std::string name;
    printColored(YELLOW_PAIR, "Enter name");
    name = readString();

    CacheManager::removeContact(name);
    changeScreen("Contacts");
}

void CLI::viewContacts() {
    std::vector<Contact> contacts = CacheManager::getContacts();
    auto contactsPage = CLI::screenMap["Contacts Page"];

    contactsPage->screenOptions.erase(
            contactsPage->screenOptions.begin() + 1,
            contactsPage->screenOptions.end());

    for (const auto& contact : contacts) {
        contactsPage->addScreenOption(
                contact.name + ": " + contact.number,
                [contact, this](){
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
    printColored(GREEN_PAIR, "Opening logs");
    system("xdg-open ./../logs/log.txt");
}

void CLI::sendUSSD() {
    WINDOW *console = createConsole("USSD console mode");
    printColored(YELLOW_PAIR, "Loading...", true, false, console);

    modem.enableUSSDConsoleMode(console);

    std::string ussd{};

    wclear(console);
    printColored(GREEN_PAIR, "Send USSD commands:", true, false, console);

    while (true) {
        ussd = readString(256, console);

        if (ussd == "exit") {
            break;
        }

        modem.sendUSSDConsoleCommand(QString::fromStdString(ussd));
    }

    modem.disableUSSDConsoleMode();
    delwin(console);
    curs_set(0);
    renderScreen();
}

void CLI::sendATCommand() {
    WINDOW *console = createConsole("AT console mode");
    printColored(YELLOW_PAIR, "Loading...", true, false, console);

    modem.enableATConsoleMode(console);

    std::string at{};

    wclear(console);
    printColored(GREEN_PAIR, "Send AT commands:", true, false, console);

    while (true) {
        at = readString(256, console);

        if (at == "exit") {
            break;
        }
        modem.sendATConsoleCommand(QString::fromStdString(at));
    }

    modem.disableATConsoleMode();
    delwin(console);
    curs_set(0);
    renderScreen();
}

void CLI::disableATConsole() {
    gotoParentScreen();
}

void CLI::prepareScreens() {
    auto mainScreen = SCREEN_SHARED_PTR("Main", nullptr);
    auto incomingCallScreen = SCREEN_SHARED_PTR("Incoming Call", mainScreen);
    auto phoneScreen = SCREEN_SHARED_PTR("Phone", mainScreen);
    auto callScreen = SCREEN_SHARED_PTR("Call", phoneScreen);
    auto inCallScreen = SCREEN_SHARED_PTR("In Call", callScreen);
    auto contactsScreen = SCREEN_SHARED_PTR("Contacts", phoneScreen);
    auto contactsPageScreen = SCREEN_SHARED_PTR("Contacts Page", contactsScreen);
    auto smsScreen = SCREEN_SHARED_PTR("SMS", phoneScreen);
    auto sendSMSScreen = SCREEN_SHARED_PTR("Send SMS", smsScreen);
    auto logScreen = SCREEN_SHARED_PTR("Logs", mainScreen);
    auto atScreen = SCREEN_SHARED_PTR("AT Console", mainScreen);
    auto ussdScreen = SCREEN_SHARED_PTR("USSD Console", mainScreen);

    mainScreen->addScreenOption("Exit", []() {
        releaseScreen();
        exit(0);
    });
    mainScreen->addScreenOption("Phone", CHANGE_SCREEN("Phone"));
    mainScreen->addScreenOption("AT Console", CHANGE_SCREEN("AT Console"));
    mainScreen->addScreenOption("USSD Console", CHANGE_SCREEN("USSD Console"));
    mainScreen->addScreenOption("Logs", CHANGE_SCREEN("Logs"));

    incomingCallScreen->addScreenOption("Reject call", EXECUTE_METHOD(rejectCall));

    incomingCallScreen->addScreenOption("Answer", EXECUTE_METHOD(answerCall));

    phoneScreen->addScreenOption("Back", GO_BACK);
    phoneScreen->addScreenOption("Call", CHANGE_SCREEN("Call"));
    phoneScreen->addScreenOption("SMS", CHANGE_SCREEN("SMS"));
    phoneScreen->addScreenOption("Contacts", CHANGE_SCREEN("Contacts"));

    callScreen->addScreenOption("Return", GO_BACK);
    callScreen->addScreenOption("Make Call", EXECUTE_METHOD(call));
    callScreen->addScreenOption("Call History", EXECUTE_METHOD(viewCallHistory));

    inCallScreen->addScreenOption("Hang up", EXECUTE_METHOD(hangUp));

    contactsScreen->addScreenOption("Back", GO_BACK);
    contactsScreen->addScreenOption("View Contacts", EXECUTE_METHOD(viewContacts));
    contactsScreen->addScreenOption("Add Contact", EXECUTE_METHOD(addContact));

    contactsPageScreen->addScreenOption("Back", GO_BACK);

    smsScreen->addScreenOption("Back", GO_BACK);
    smsScreen->addScreenOption("Messages", EXECUTE_METHOD(viewMessages));
    smsScreen->addScreenOption("Send SMS", CHANGE_SCREEN("Send SMS"));
    sendSMSScreen->addScreenOption("Back", GO_BACK);
    sendSMSScreen->addScreenOption("Write SMS", EXECUTE_METHOD(sendMessage));

    logScreen->addScreenOption("Back", GO_BACK);
    logScreen->addScreenOption("View Logs", EXECUTE_METHOD(viewLogs));

    ussdScreen->addScreenOption("Back", GO_BACK);
    ussdScreen->addScreenOption("Send USSD Command", EXECUTE_METHOD(ussdConsoleMode));

    atScreen->addScreenOption("Back", EXECUTE_METHOD(disableATConsole));
    atScreen->addScreenOption("Send AT Command", EXECUTE_METHOD(atConsoleMode));


    CLI::screenMap = {
            {"Main",          mainScreen},
            {"Incoming Call", incomingCallScreen},
            {"Phone",         phoneScreen},
            {"Call",          callScreen},
            {"In Call",       inCallScreen},
            {"Contacts",      contactsScreen},
            {"Contacts Page", contactsPageScreen},
            {"SMS",           smsScreen},
            {"Send SMS",      sendSMSScreen},
            {"Logs",          logScreen},
            {"USSD Console",  ussdScreen},
            {"AT Console",    atScreen}
    };

    currentScreen = mainScreen;
}
