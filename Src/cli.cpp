//
// Created by paul on 1/14/23.
//

#include "../Inc/cli.h"

CLI::CLI(Modem &modem) : modem(modem) {
    connect(&modem, SIGNAL(callStatusChanged()), this, SLOT(handleIncomingCall()));
    connect(&modem, SIGNAL(newMessage()), this, SLOT(handleIncomingSMS()));
}

void CLI::handleIncomingCall() const {
    qDebug() << modem.callStatus;
    qDebug() << "Incoming call";
}

void CLI::handleIncomingSMS() {
    qDebug() << "Incoming SMS";
}