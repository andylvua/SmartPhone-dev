//
// Created by paul on 1/14/23.
//

#include "logging.hpp"
#include "cli/cli.hpp"
#include "cli/definitions/cli_macros.hpp"
#include "cli/utils/ncurses_utils.hpp"
#include "modem/utils/cache_manager.hpp"
#include "cli/utils/cli_utils.hpp"
#include <QProcess>
#include <string>

const auto cliLogger = spdlog::basic_logger_mt("cli", "../logs/log.txt", true);

CLI::CLI(Modem &modem) : modem(modem) {
    prepareScreens();

#ifdef BUILD_ON_RASPBERRY
#include "rotary_reader/rotary_dial.hpp"
    RotaryDial::setup();
#endif

    connect(&modem, SIGNAL(incomingCall(QString)),
            this, SLOT(handleIncomingCall(QString)));
    connect(&modem, SIGNAL(incomingSMS()), this, SLOT(handleIncomingSMS()));
    connect(&modem, SIGNAL(callEnded()), this, SLOT(handleCallEnded()));
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

void CLI::incrementActivePage() const {
    int optionsPerPage = currentScreen->getMaxOptionsPerPage();
    if (!currentScreen->isLastPage()) {
        currentScreen->activeOption += optionsPerPage - currentScreen->getActiveOption() % optionsPerPage;
    }
    updateScreen();
}

void CLI::decrementActivePage() const {
    int optionsPerPage = currentScreen->getMaxOptionsPerPage();
    if (!currentScreen->isFirstPage()) {
        currentScreen->activeOption -= optionsPerPage + currentScreen->getActiveOption() % optionsPerPage;
        updateScreen();
    }
}

void CLI::listen() const {
    cliLogger->flush_on(spdlog::level::debug);
    SPDLOG_LOGGER_INFO(cliLogger, "CLI listener started.");

    NcursesUtils::initScreen();
    renderScreen();

    int ch;

    while (true) {
        if ((ch = getch()) == 27) { // ESC
            NcursesUtils::releaseScreen();
            exit(0);
        }

        switch (ch) {
            case KEY_UP:
                CLI::decrementActiveOption();
                break;
            case KEY_DOWN:
                CLI::incrementActiveOption();
                break;
            case KEY_LEFT:
                CLI::decrementActivePage();
                break;
            case KEY_RIGHT:
                CLI::incrementActivePage();
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
    auto debugSettingsScreen = SCREEN_SHARED_PTR("Debug Settings", settingsScreen);
    auto aboutScreen = SCREEN_SHARED_PTR("About Device", settingsScreen);

    mainScreen->addScreenOption("Exit", []() {
        NcursesUtils::releaseScreen();
        exit(0);
    });
    mainScreen->addScreenOption("Phone", CHANGE_SCREEN("Phone"));
    mainScreen->addScreenOption("AT Console", CHANGE_SCREEN("AT Console"));
    mainScreen->addScreenOption("USSD Console", CHANGE_SCREEN("USSD Console"));
    mainScreen->addScreenOption("HTTP Console", CHANGE_SCREEN("HTTP Console"));
    mainScreen->addScreenOption("Settings", CHANGE_SCREEN("Settings"));
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

    atScreen->addScreenOption("Back", GO_BACK);
    atScreen->addScreenOption("Send AT Command", EXECUTE_METHOD(atConsoleMode));

    httpScreen->addScreenOption("Back", GO_BACK);
    httpScreen->addScreenOption("Send HTTP Command", EXECUTE_METHOD(httpConsoleMode));

    settingsScreen->addScreenOption("Back", GO_BACK);
    settingsScreen->addScreenOption("Debug Settings", CHANGE_SCREEN("Debug Settings"));
    settingsScreen->addScreenOption("About Device", EXECUTE_METHOD(aboutDevice));

    debugSettingsScreen->addScreenOption("Back", GO_BACK);
    debugSettingsScreen->addScreenOption("Number Identifier", EXECUTE_METHOD(setNumberID), true);
    debugSettingsScreen->addScreenOption("Text Mode", EXECUTE_METHOD(setMessageMode), true);
    debugSettingsScreen->addScreenOption("Echo Mode", EXECUTE_METHOD(setEchoMode), true);

    aboutScreen->addScreenOption("Back", GO_BACK);

    CLI::screenMap = {
            {"Main",           mainScreen},
            {"Incoming Call",  incomingCallScreen},
            {"Phone",          phoneScreen},
            {"Call",           callScreen},
            {"In Call",        inCallScreen},
            {"Contacts",       contactsScreen},
            {"Contacts Page",  contactsPageScreen},
            {"SMS",            smsScreen},
            {"Send SMS",       sendSMSScreen},
            {"Logs",           logScreen},
            {"USSD Console",   ussdScreen},
            {"AT Console",     atScreen},
            {"HTTP Console",   httpScreen},
            {"AT Console",     atScreen},
            {"Settings",       settingsScreen},
            {"Debug Settings", debugSettingsScreen},
            {"About Device",   aboutScreen}
    };

    currentScreen = mainScreen;
}
