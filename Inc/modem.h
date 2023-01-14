//
// Created by Andrew Yaroshevych on 21.12.2022.
//

#ifndef UNTITLED3_MODEM_H
#define UNTITLED3_MODEM_H

#include "serial.h"

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


class Modem {
public:
    SerialPort &serial;
    commLineState_t commLineStatus;
    callState_t callStatus;

    explicit Modem(SerialPort &serial);

    commLineState_t getCommLineStatus() const;

    void setCommLineStatus(commLineState_t commLineStatus);

    bool checkAT();

    bool checkRegistration();

    bool call(const std::string& number);

    bool hangUp();

    bool message(const std::string& number, const std::string& message);

    bool initialize();
};
#endif //UNTITLED3_MODEM_H
