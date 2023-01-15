//
// Created by paul on 1/14/23.
//

#include "../Inc/cli.h"

CLI::CLI(Modem &modem) : modem(modem) {
    connect(&modem, SIGNAL(incomingCall()), this, SLOT(handleIncomingCall()));
    connect(&modem, SIGNAL(incomingSMS()), this, SLOT(handleIncomingSMS()));
    connect(this, SIGNAL(requestCall(QString)), &modem, SLOT(performCall(QString)));
}

void CLI::handleIncomingCall() const {
    qDebug() << modem.callStatus;
    qDebug() << "Incoming call";
}

void CLI::handleIncomingSMS() {
    qDebug() << "Incoming SMS";
    modem.checkAT();
}

void CLI::listen() {
char *line;
    while ((line = readline(">>> "))) {
        if (line[0] == '\0') {
            continue;
        }

        add_history(line);

        if (strcmp(line, "exit") == 0) {
            break;
        }

        if (strcmp(line, "call") == 0) {
            qDebug() << "Requesting call";
            emit requestCall("+380679027620");
        }
    }
}