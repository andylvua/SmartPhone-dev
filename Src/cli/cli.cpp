//
// Created by paul on 1/14/23.
//

#include "../../Inc/logging.hpp"
#include "../../Inc/cli/cli.hpp"
#include "../../Inc/cli/cli_macros.hpp"
#include "../../Inc/cli/ncurses_utils.hpp"
#include "../../Inc/modem/cache_manager.hpp"
#include "../../Inc/cli/colors.hpp"
#include "../../Inc/cli/readline_utils.hpp"
#include <readline/readline.h>
#include <readline/history.h>
#include <QProcess>
#include <string>
#include <fstream>
#include <regex>

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

        printColored(color, option->optionName.toStdString());
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

        switch (ch) {
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
                currentScreen->screenOptions[currentScreen->getActiveOption()]->execute();
                break;
            default:
                break;
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

void CLI::ussdConsoleMode() {
    curs_set(1);
    releaseScreen();
    system("clear");

    std::cout << YELLOW_COLOR << "Loading..." << RESET;
    std::cout.flush();

    modem.enableUSSDConsoleMode();

    const char *ussd;

    std::cout << "\r" << GREEN_COLOR << "Send USSD commands. Type 'exit' to exit" << RESET << std::endl;

    while ((ussd = readline("")) != nullptr) {
        if (*ussd) {
            add_history(ussd);
        }
        if (strcmp(ussd, "exit") == 0) {
            break;
        }
        if (strcmp(ussd, "clear") == 0) {
            system("clear");
            continue;
        }

        modem.sendUSSDConsoleCommand(ussd);

        free((void *) ussd);
    }

    modem.disableUSSDConsoleMode();

    initScreen();
    renderScreen();
}

void CLI::atConsoleMode() {
    curs_set(1);
    releaseScreen();
    system("clear");

    std::cout << YELLOW_COLOR << "Loading..." << RESET;
    std::cout.flush();

    modem.enableATConsoleMode();

    initReadlineCompletions();
    rl_attempted_completion_function = commandCompletion;

    const char *at;

    std::cout << "\r" << GREEN_COLOR << "Send AT commands. Type exit to exit:" << RESET << std::endl;

    while ((at = readline("")) != nullptr) {
        if (*at) {
            add_history(at);
        }
        if (strcmp(at, "exit") == 0) {
            break;
        }
        if (strcmp(at, "clear") == 0) {
            system("clear");
            continue;
        }

        modem.sendATConsoleCommand(at);

        free((void *) at);
    }

    modem.disableATConsoleMode();

    initScreen();
    renderScreen();
}

std::string parseUrl(std::string httpCommand, httpMethod_t method) {
    std::string url;
    url = httpCommand.erase(0, httpCommand.find(' ') + 1);

    if (!url.starts_with("http://") && !url.starts_with("https://")) {
        url = "https://" + url;
    }

    url.erase(0, url.find_first_not_of(' '));
    url.erase(url.find_last_not_of(' ') + 1);

    std::basic_regex<char> urlRegex(
            "^(http|https)://[a-zA-Z0-9-]+(\\.[a-zA-Z0-9-]+)+([a-zA-Z0-9-._~:/?#[\\]@!$&'()*+,;=]*)?$"
            );
    if (!std::regex_match(url, urlRegex)) {
        std::cout << RED_COLOR << "Invalid URL" << RESET << std::endl;
        return "";
    }

    return url;
}

void CLI::httpConsoleMode() {
    curs_set(1);
    releaseScreen();
    system("clear");

    std::cout << YELLOW_COLOR << "Loading..." << RESET;
    std::cout.flush();

    auto consoleEnabled = modem.enableHTTPConsoleMode();

    if (!consoleEnabled) {
        modem.disableHTTPConsoleMode();

        initScreen();
        renderScreen();
        return;
    }

    const char *http;

    std::cout << "\r" << GREEN_COLOR
    << "Send HTTP commands. GET and POST are supported. Type exit to exit:" << RESET << std::endl;

    while ((http = readline("")) != nullptr) {
        if (*http) {
            add_history(http);
        }
        if (strcmp(http, "exit") == 0) {
            break;
        }
        if (strcmp(http, "clear") == 0) {
            system("clear");
            continue;
        }

        httpMethod_t method;

        if (strstr(http, "GET ") != nullptr) {
            method = httpMethod::HM_GET;
        } else if (strstr(http, "POST ") != nullptr) {
            method = httpMethod::HM_POST;
        } else {
            std::cout << RED_COLOR << "Invalid HTTP command" << RESET << std::endl;
            continue;
        }

        std::string rawCommand = http;
        std::string url = parseUrl(rawCommand, method);

        if (url.empty()) {
            SPDLOG_LOGGER_INFO(cliLogger, "Invalid HTTP command: {}", rawCommand);
            continue;
        }

        SPDLOG_LOGGER_INFO(cliLogger, "Sending HTTP command: {}", url);
        modem.sendHTTPConsoleCommand(QString::fromStdString(url), method);

        free((void *) http);
    }

    modem.disableHTTPConsoleMode();

    initScreen();
    renderScreen();
}

void CLI::setMessageMode() {
    for (const auto& option: CLI::screenMap["Debug Mode"]->screenOptions) {
        if (option->optionName == "Message Mode" && option->isSwitcher) {
            modem.setMessageMode(!option->getState());
            option->switchState();
        }
    }
    updateScreen();
}
void CLI::setNumberID() {
    for (const auto& option: CLI::screenMap["Debug Mode"]->screenOptions) {
        if (option->optionName == "Number Identifier" && option->isSwitcher) {
            modem.setNumberID(!option->getState());
            option->switchState();
        }
    }
    updateScreen();
}

void CLI::setEchoMode(){
    for (const auto& option: CLI::screenMap["Debug Mode"]->screenOptions) {
        if (option->optionName == "Echo Mode" && option->isSwitcher){
            modem.setEchoMode(!option->getState());
            option->switchState();
        }
    }
    updateScreen();
}

void CLI::setPIN() {
    std::string pin;
    printColored(YELLOW_PAIR, "Enter PIN: ");
    pin = readString();
    modem.setPIN(pin);
}

void CLI::aboutDevice(){
    changeScreen("About Device");
    QString aboutInfo = modem.aboutDevice();
    printColored(WHITE_PAIR, aboutInfo.toStdString());
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
    auto httpScreen = SCREEN_SHARED_PTR("HTTP Console", mainScreen);
    auto settingsScreen = SCREEN_SHARED_PTR("Settings", mainScreen);
    auto debugSettingsScreen = SCREEN_SHARED_PTR("Debug Mode", settingsScreen);
    auto simSettingsScreen = SCREEN_SHARED_PTR("SIM Settings", settingsScreen);
    auto aboutScreen = SCREEN_SHARED_PTR("About Device", settingsScreen);

    mainScreen->addScreenOption("Exit", []() {
        releaseScreen();
        exit(0);
    });
    mainScreen->addScreenOption("Phone", CHANGE_SCREEN("Phone"));
    mainScreen->addScreenOption("AT Console", CHANGE_SCREEN("AT Console"));
    mainScreen->addScreenOption("USSD Console", CHANGE_SCREEN("USSD Console"));
    mainScreen->addScreenOption("HTTP Console", CHANGE_SCREEN("HTTP Console"));
    mainScreen->addScreenOption("Logs", CHANGE_SCREEN("Logs"));
    mainScreen->addScreenOption("Settings", CHANGE_SCREEN("Settings"));


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

    atScreen->addScreenOption("Back", GO_BACK);
    atScreen->addScreenOption("Send AT Command", EXECUTE_METHOD(atConsoleMode));

    httpScreen->addScreenOption("Back", GO_BACK);
    httpScreen->addScreenOption("Send HTTP Command", EXECUTE_METHOD(httpConsoleMode));
    settingsScreen->addScreenOption("Back", GO_BACK);
    settingsScreen->addScreenOption("Debug Mode", CHANGE_SCREEN("Debug Mode"));
    settingsScreen->addScreenOption("SIM Settings", CHANGE_SCREEN("SIM Settings"));
    settingsScreen->addScreenOption("About Device", EXECUTE_METHOD(aboutDevice));

    debugSettingsScreen->addScreenOption("Back", GO_BACK);
    debugSettingsScreen->addScreenOption("Message Mode", EXECUTE_METHOD(setMessageMode),
                                         true, true);
    debugSettingsScreen->addScreenOption("Number Identifier", EXECUTE_METHOD(setNumberID),
                                         true, true);
    debugSettingsScreen->addScreenOption("Echo Mode", EXECUTE_METHOD(setEchoMode),
                                         true, true);

    simSettingsScreen->addScreenOption("Back", GO_BACK);
    simSettingsScreen->addScreenOption("Set PIN", EXECUTE_METHOD(setPIN));

    aboutScreen->addScreenOption("Back", GO_BACK);

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
            {"AT Console",    atScreen},
            {"HTTP Console",  httpScreen},
            {"AT Console",    atScreen},
            {"Settings",      settingsScreen},
            {"Debug Mode",    debugSettingsScreen},
            {"SIM Settings",  simSettingsScreen},
            {"About Device",  aboutScreen}
    };

    currentScreen = mainScreen;
}
