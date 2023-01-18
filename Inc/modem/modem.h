//
// Created by Andrew Yaroshevych on 21.12.2022.
//

#ifndef UNTITLED3_MODEM_H
#define UNTITLED3_MODEM_H

#include "serial.h"
#include <QDateTime>
#include <string>
#include <utility>
#include <iostream>

constexpr const char* CONTACTS_FILEPATH = "../module_cache/contacts.txt";
constexpr const char* MESSAGES_FILEPATH = "../module_cache/messages.txt";
constexpr const char* CALLS_FILEPATH = "../module_cache/calls.txt";

using callResult_t = enum class callResult {
    CR_ANSWERED,
    CR_NO_ANSWER
};

using callDirection_t = enum class callDirection {
    CD_INCOMING,
    CD_OUTGOING
};

struct Call {
    callResult_t callResult;
    callDirection_t callDirection;
    QString number;
    QDateTime startTime;
    QDateTime endTime;
};

class Modem : public QObject {
Q_OBJECT
public:
    SerialPort &serial;
    Call currentCall;

    bool workerStatus = false;
    bool consoleMode = false;
    __attribute__((unused)) std::ostream &outStream = std::cout;

    explicit Modem(SerialPort &serial);

    void sendConsoleCommand(const QString &command);

    void enableConsoleMode();

    void disableConsoleMode();

    static QString parseLine(const QByteArray &line);

    bool checkAT();

    bool checkRegistration();

    bool call(const std::string &number);

    bool hangUp();

    bool answer();

    bool message(const std::string &number, const std::string &message);

    bool initialize();

    void worker();

    QByteArray readLine();

    void dataInterruptHandler();

    void listen();

    static void addContact(const std::string &name, const std::string &number);

    static void removeContact(const std::string &name);

    static void listContacts();

    static void listMessages();

signals:

    void incomingCall(QString number);

    void incomingSMS();

    void callEnded();

private:

    void _ringHandler(const QString &parsedLine);

    void _ciev_call_1Handler();

    void _ciev_call_0Handler(const QString &parsedLine);

    void _sounder_0Handler();

    void _message_1Handler(const QString &parsedLine);
};

#endif //UNTITLED3_MODEM_H
