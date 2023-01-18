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
    Screen *currentScreen{};
    std::vector<Screen*> screens;

    void renderScreen();

    void changeScreen(Screen *screen);

    void changeScreen(const QString &screenName);

    void listen();

    void addScreen(Screen *screen);


    void mainScreenHandler(char* line);

    void incomingCallScreenHandler(char* line);

    void phoneScreenHandler(char* line);

    void callScreenHandler(char* line);

    void inCallScreenHandler(char* line);

    void contactsScreenHandler(char* line);

    void smsScreenHandler(char* line);

    void sendSMSScreenHandler(char* line);

    void ussdScreenHandler(char* line);

    void atScreenHandler(char* line);

    void logsScreenHandler(char* line);

    void prepareScreens();

public slots:

    void handleIncomingCall(QString number);

    void handleIncomingSMS();

    void handleCallEnded();

};


#endif //PHONE_CLI_H
