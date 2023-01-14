//
// Created by paul on 1/14/23.
//

#ifndef PHONE_CLI_H
#define PHONE_CLI_H


#include <QObject>
#include "modem.h"

class CLI : public QObject {
Q_OBJECT

public:
//    CLIState state;
    CLI(Modem &modem);
    Modem &modem;

public slots:

    void handleIncomingCall() const;

    void handleIncomingSMS();

};


#endif //PHONE_CLI_H
