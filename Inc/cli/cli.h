//
// Created by paul on 1/14/23.
//

#ifndef PHONE_CLI_H
#define PHONE_CLI_H

#include <QObject>
#include <iostream>
#include "../modem/modem.h"
#include "screen.h"
#include <readline/readline.h>
#include <readline/history.h>
#include "../rotary_reader/rotary_dial.h"
#include <memory>
#include <ncurses.h>
#include <unordered_map>
#include "../../Inc/cli/option.h"

#define ScreenSharedPtr(name, parentScreen) std::make_shared<Screen>(name, parentScreen)
#define GO_BACK [&](){gotoParentScreen();}
#define CHANGE_SCREEN(screenName) [&](){changeScreen(screenName);}
#define EXECUTE_METHOD(method) [&](){method();}
using ScreenMap = std::unordered_map<QString, std::shared_ptr<Screen>>;

class CLI : public QObject {
Q_OBJECT

public:
    explicit CLI(Modem &modem);

    Modem &modem;
    std::shared_ptr<Screen> currentScreen{};
    ScreenMap screenMap;

    void incrementActiveOption() const;

    void decrementActiveOption() const;

    void renderScreen() const;

    void changeScreen(const QString &screenName);

    void gotoParentScreen();

    void listen() const;

    void prepareScreens();

public slots:

    void handleIncomingCall(const QString &number);

    void handleIncomingSMS();

    void handleCallEnded();

private:

    void _rejectCall();

    void _answerCall();

    void _viewCallHistory();

    void _call();

    void _hangUp();

    void _addContact();

    void _deleteContact();

    void _viewContacts();

    void _viewMessages();

    void _sendMessage();

    void _viewLogs();

    void _sendUSSD();

    void _sendATCommand();

    void _disableATConsole();
};

#endif //PHONE_CLI_H
