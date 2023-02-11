//
// Created by Andrew Yaroshevych on 05.02.2023.
//

#ifndef PHONE_MODEM_CONTROLLER_HPP
#define PHONE_MODEM_CONTROLLER_HPP

#include "cli/cli.hpp"
#include "modem/modem.hpp"

class ModemController : public QObject {
    CLI &cli;
    Modem &modem;

public:
    explicit ModemController(CLI &cli, Modem &modem);

    void call();

    void call(const QString &number);

    void answerCall();

    void rejectCall();

    void hangUp();

    void sendMessage();

    void sendMessage(const QString &number);

    void ussdConsoleMode();

    void atConsoleMode();

    void httpConsoleMode();

    void setMessageMode();

    void setNumberID();

    void setEchoMode();

    void aboutDevice();
};

#endif //PHONE_MODEM_CONTROLLER_HPP
