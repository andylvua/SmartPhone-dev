//
// Created by Andrew Yaroshevych on 21.12.2022.
//

#ifndef UNTITLED3_MODEM_H
#define UNTITLED3_MODEM_H

#include "serial.h"
#include <QDateTime>
#include <string>
#include <utility>

typedef enum {
    CLS_FREE,
    CLS_ATCMD,
    CLS_DATA
} commLineState_t;

typedef enum {
    CS_IDLE,
    CS_ACTIVE,
    CS_HELD,
    CS_DIALING,
    CS_ALERTING,
    CS_INCOMING,
    CS_WAITING
} callState_t;

typedef enum {
    CR_ANSWERED,
    CR_NO_ANSWER
} callResult_t;

typedef enum {
    CD_INCOMING,
    CD_OUTGOING
} callDirection_t;

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
    commLineState_t commLineStatus;
    callState_t callStatus;
    Call currentCall;

    bool workerStatus = false;

    explicit Modem(SerialPort &serial);

    commLineState_t getCommLineStatus() const;

    void setCommLineStatus(commLineState_t commLineStatus);

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

    void incomingCall();

    void incomingSMS();

    void callEnded();

public slots:


private:

    void _ringHandler(const QString &parsedLine);

    void _ciev_call_1Handler();

    void _ciev_call_0Handler(const QString &parsedLine);

    void _sounder_0Handler();

    void _message_1Handler(const QString &parsedLine);
};

#endif //UNTITLED3_MODEM_H