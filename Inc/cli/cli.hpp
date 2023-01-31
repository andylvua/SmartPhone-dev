//
// Created by paul on 1/14/23.
//

#ifndef PHONE_CLI_HPP
#define PHONE_CLI_HPP

#include "../modem/modem.hpp"
#include "../rotary_reader/rotary_dial.hpp"
#include "../../Inc/cli/option.hpp"

#include <QObject>
#include <iostream>
#include "screen.hpp"
#include <memory>
#include <ncurses.h>
#include <unordered_map>

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

    void updateScreen() const;

    void changeScreen(const QString &screenName);

    void gotoParentScreen();

    void listen() const;

    void prepareScreens();

    void viewContacts();

    void call(const QString &number);

    void sendMessage(const QString &number);

public slots:

    void handleIncomingCall(const QString &number);

    void handleIncomingSMS();

    void handleCallEnded();

private:

    void rejectCall();

    void answerCall();

    void viewCallHistory() const;

    void call();

    void hangUp();

    void addContact();

    void viewMessages();

    void sendMessage();

    static void viewLogs();

    void ussdConsoleMode();

    void atConsoleMode();

    void httpConsoleMode();

    void setMessageMode();

    void setNumberID();

    void setEchoMode();

    void setPIN();

    void aboutDevice();

};

#endif //PHONE_CLI_HPP
