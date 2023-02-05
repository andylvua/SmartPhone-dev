//
// Created by Andrew Yaroshevych on 06.02.2023.
//

#include "cli/cli.hpp"
#include "cli/modem_controller.hpp"
#include "cli/definitions/cli_macros.hpp"
#include "cli/utils/ncurses/ncurses_utils.hpp"

void CLI::initScreens() {
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


    incomingCallScreen->addScreenOption("Reject call", EXECUTE_METHOD(modemController->rejectCall));
    incomingCallScreen->addScreenOption("Answer", EXECUTE_METHOD(modemController->answerCall));

    phoneScreen->addScreenOption("Back", GO_BACK);
    phoneScreen->addScreenOption("Call", CHANGE_SCREEN("Call"));
    phoneScreen->addScreenOption("SMS", CHANGE_SCREEN("SMS"));
    phoneScreen->addScreenOption("Contacts", CHANGE_SCREEN("Contacts"));

    callScreen->addScreenOption("Return", GO_BACK);
    callScreen->addScreenOption("Make Call", EXECUTE_METHOD(modemController->call));
    callScreen->addScreenOption("Call History", EXECUTE_METHOD(viewCallHistory));

    inCallScreen->addScreenOption("Hang up", EXECUTE_METHOD(modemController->hangUp));

    contactsScreen->addScreenOption("Back", GO_BACK);
    contactsScreen->addScreenOption("View Contacts", EXECUTE_METHOD(viewContacts));
    contactsScreen->addScreenOption("Add Contact", EXECUTE_METHOD(addContact));

    contactsPageScreen->addScreenOption("Back", GO_BACK);

    smsScreen->addScreenOption("Back", GO_BACK);
    smsScreen->addScreenOption("Messages", EXECUTE_METHOD(viewMessages));
    smsScreen->addScreenOption("Send SMS", CHANGE_SCREEN("Send SMS"));
    sendSMSScreen->addScreenOption("Back", GO_BACK);
    sendSMSScreen->addScreenOption("Write SMS", EXECUTE_METHOD(modemController->sendMessage));

    logScreen->addScreenOption("Back", GO_BACK);
    logScreen->addScreenOption("View Logs", EXECUTE_METHOD(viewLogs));

    ussdScreen->addScreenOption("Back", GO_BACK);
    ussdScreen->addScreenOption("Send USSD Command", EXECUTE_METHOD(modemController->ussdConsoleMode));

    atScreen->addScreenOption("Back", GO_BACK);
    atScreen->addScreenOption("Send AT Command", EXECUTE_METHOD(modemController->atConsoleMode));

    httpScreen->addScreenOption("Back", GO_BACK);
    httpScreen->addScreenOption("Send HTTP Command", EXECUTE_METHOD(modemController->httpConsoleMode));

    settingsScreen->addScreenOption("Back", GO_BACK);
    settingsScreen->addScreenOption("Debug Settings", CHANGE_SCREEN("Debug Settings"));
    settingsScreen->addScreenOption("About Device", EXECUTE_METHOD(modemController->aboutDevice));

    debugSettingsScreen->addScreenOption("Back", GO_BACK);
    debugSettingsScreen->addScreenOption("Number Identifier",
                                         EXECUTE_METHOD(modemController->setNumberID), true);
    debugSettingsScreen->addScreenOption("Text Mode",
                                         EXECUTE_METHOD(modemController->setMessageMode), true);
    debugSettingsScreen->addScreenOption("Echo Mode",
                                         EXECUTE_METHOD(modemController->setEchoMode), true);

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
