//
// Created by Andrew Yaroshevych on 21.12.2022.
//

#ifndef PHONE_MODEM_HPP
#define PHONE_MODEM_HPP

#include "serial.hpp"
#include "media_types.hpp"
#include <QDateTime>
#include <iostream>
#include <ncurses.h>

using consoleType_t = enum class consoleType {
    CM_AT,
    CM_USSD
};

struct ConsoleMode {
    bool enabled;
    consoleType_t consoleType;
};

class Modem : public QObject {
Q_OBJECT
public:
    SerialPort &serial;

    bool workerStatus = false;

    ConsoleMode consoleMode{};

    Call currentCall;

    std::ostream &outStream = std::cout;

    explicit Modem(SerialPort &serial);

    QByteArray readLine();

    static QString parseLine(const QByteArray &line);

    bool enableEcho();

    void enableATConsoleMode();

    void disableATConsoleMode();

    void enableUSSDConsoleMode();

    void disableUSSDConsoleMode();

    void sendATConsoleCommand(const QString &command);

    void sendUSSDConsoleCommand(const QString &command);

    void setCharacterSet(const std::string &characterSet);

    bool checkAT();

    bool checkRegistration();

    bool call(const std::string &number);

    bool hangUp();

    bool answer();

    bool message(const std::string &number, const std::string &message);

    bool setMessageMode(bool mode);

    bool setNumberID(bool mode);

    bool setEchoMode(bool mode);

    bool setPIN(const std::string &pin);

    QString aboutDevice();

    bool initialize();

    void worker();

    void dataInterruptHandler();

    void listen();

signals:

    void incomingCall(QString number);

    void incomingSMS();

    void callEnded();

private:

    void ringHandler(const QString &parsedLine);

    void cievCall1Handler();

    void cievCall0Handler(const QString &parsedLine);

    void sounder0Handler();

    void message1Handler(const QString &parsedLine);

    void atConsoleMode();

    void ussdConsoleMode();
};

#endif //PHONE_MODEM_HPP
