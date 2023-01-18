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

class CLI : public QObject {
Q_OBJECT

public:
    explicit CLI(Modem &modem);

    Modem &modem;
    std::shared_ptr<Screen> currentScreen{};
    std::vector<std::shared_ptr<Screen>> screens;

    void renderScreen() const;

    void changeScreen(std::shared_ptr<Screen> screen);

    void changeScreen(const QString &screenName);

    void listen();

    void mainScreenHandler(const char* line);

    void incomingCallScreenHandler(const char* line);

    void phoneScreenHandler(const char* line);

    void callScreenHandler(const char* line);

    void inCallScreenHandler(const char* line);

    void contactsScreenHandler(const char* line);

    void smsScreenHandler(const char* line);

    void sendSMSScreenHandler(const char* line);

    void ussdScreenHandler(const char* line);

    void atScreenHandler(const char* line);

    void logsScreenHandler(const char* line);

    void prepareScreens();

public slots:

    void handleIncomingCall(const QString& number);

    void handleIncomingSMS();

    void handleCallEnded();

};

#endif //PHONE_CLI_H
