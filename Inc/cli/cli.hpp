//
// Created by paul on 1/14/23.
//

#ifndef PHONE_CLI_HPP
#define PHONE_CLI_HPP

#include "modem/modem.hpp"
#include "rotary_reader/rotary_dial.hpp"
#include "cli/option.hpp"

#include <QObject>
#include <iostream>
#include "screen.hpp"
#include <memory>
#include <ncurses.h>
#include <unordered_map>

using ScreenMap = QHash<QString, QSharedPointer<Screen>>;

class CLI : public QObject {
Q_OBJECT

public:
    explicit CLI(Modem &modem);

    Modem &modem;
    QSharedPointer<Screen> currentScreen;
    ScreenMap screenMap;

    void listen() const;

    void gotoParentScreen();

    void viewContacts();

    void call();

    void call(const QString &number);

    void sendMessage();

    void sendMessage(const QString &number);

public slots:

    void handleIncomingCall(const QString &number);

    void handleIncomingSMS();

    void handleCallEnded();

protected:
    void changeScreen(const QString &screenName);

private:
    static void render(const QSharedPointer<Screen> &screen) ;

    void prepareScreens();

    void incrementActiveOption() const;

    void decrementActiveOption() const;

    void incrementActivePage() const;

    void decrementActivePage() const;

    void renderScreen() const;

    void updateScreen() const;

    static void disableNcursesScreen();

    void enableNcursesScreen();

    void rejectCall();

    void answerCall();

    void hangUp();

    void addContact();

    void viewCallHistory() const;

    void viewMessages();

    void viewLogs() const;

    void ussdConsoleMode();

    void atConsoleMode();

    void httpConsoleMode();

    void setMessageMode();

    void setNumberID();

    void setEchoMode();

    void aboutDevice();

};

#endif //PHONE_CLI_HPP
