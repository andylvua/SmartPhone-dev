//
// Created by paul on 1/14/23.
//

#ifndef PHONE_CLI_H
#define PHONE_CLI_H


#include <QObject>
#include "modem.h"
#include <readline/readline.h>
#include <readline/history.h>

class CLI : public QObject {
Q_OBJECT

public:
    explicit CLI(Modem &modem);

    Modem &modem;

    void listen();

signals:

    void requestCall(const QString &number);

public slots:

    void handleIncomingCall() const;

    void handleIncomingSMS();

};


#endif //PHONE_CLI_H
