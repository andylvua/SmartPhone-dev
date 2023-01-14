//
// Created by Andrew Yaroshevych on 21.12.2022.
//

#ifndef UNTITLED3_MODEM_H
#define UNTITLED3_MODEM_H

#include "serial.h"
#include <chrono>
#include <ctime>
#include <QDateTime>
#include <utility>

// commLineState
typedef enum {
    CLS_FREE,
    CLS_ATCMD,
    CLS_DATA
} commLineState_t;

// callState
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

class Modem {
public:
    SerialPort &serial;
    commLineState_t commLineStatus;
    callState_t callStatus;
    Call currentCall;
    bool newSMS;

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

    void listen();
};

#endif //UNTITLED3_MODEM_H