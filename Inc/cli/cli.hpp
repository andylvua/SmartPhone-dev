//
// Created by paul on 1/14/23.
//

#ifndef PHONE_CLI_HPP
#define PHONE_CLI_HPP

#include <QObject>
#include <QMainWindow>
#include <iostream>
#include <memory>
#include <ncurses.h>
#include <unordered_map>
#include <QSharedPointer>
#include "screen.hpp"
#include "cli/option.hpp"
#include "rotary_reader/rotary_dial.hpp"

using ScreenMap = QHash<QString, QSharedPointer<Screen>>;

class ModemController;

class CLI : public QObject {
Q_OBJECT

public:
    explicit CLI();

    ModemController *modemController;
    QSharedPointer<Screen> currentScreen;
    ScreenMap screenMap;

    void setModemController(ModemController *modemController);

    void listen() const;

    void renderScreen() const;

    void updateScreen() const;

    void changeScreen(const QString &screenName);

    void gotoParentScreen();

    void enableNcursesScreen() const;

    static void disableNcursesScreen();

    void addContact();

    void viewContacts();

    void viewCallHistory() const;

    void viewMessages();

    void viewLogs() const;

public slots:

    void handleIncomingCall(const QString &number);

    void handleIncomingSMS();

    void handleCallEnded();

private:

    void initScreens();

    void incrementActiveOption() const;

    void decrementActiveOption() const;

    void incrementActivePage() const;

    void decrementActivePage() const;
};

#endif //PHONE_CLI_HPP
