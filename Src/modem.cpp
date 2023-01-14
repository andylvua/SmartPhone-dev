//
// Created by Andrew Yaroshevych on 21.12.2022.
//

#include "../Inc/modem.h"
#include "../Inc/command.h"
#include "../Inc/commands_list.h"
#include <iostream>

Modem::Modem(SerialPort &serial) : serial(serial) {
    commLineStatus = CLS_FREE;
    callStatus = CS_IDLE;
}

commLineState_t Modem::getCommLineStatus() const {
    return commLineStatus;
}

void Modem::setCommLineStatus(commLineState_t commLineStatusSet) {
    Modem::commLineStatus = commLineStatusSet;
}

bool Modem::checkAT() {
    if (commLineStatus == CLS_FREE) {
        commLineStatus = CLS_ATCMD;
        GetCommand command = GetCommand(AT, serial);
        auto response = command.execute();
        commLineStatus = CLS_FREE;

        if (response.indexOf("OK") != -1) {
            return true;
        }

        return false;
    } else {
        return false;
    }
}

bool Modem::checkRegistration() {
    if (commLineStatus == CLS_FREE) {
        commLineStatus = CLS_ATCMD;
        GetCommand command = GetCommand(AT_CREG"?", serial);
        auto response = command.execute();
        commLineStatus = CLS_FREE;

        if (response.indexOf("+CREG: 0,1") != -1
            || (response.indexOf("+CREG: 0,5") != -1)
            || (response.indexOf("+CREG: 1,1") != -1)
            || (response.indexOf("+CREG: 1,5") != -1)) {
            return true;
        }
        return false;
    } else {
        return false;
    }
}

bool Modem::call(const std::string& number) {
    if (commLineStatus != CLS_FREE) {
        return false;
    }

    commLineStatus = CLS_ATCMD;
    Task task(ATD + number + ";", serial);
    commRes_t res = task.execute();

    if (res != CR_OK) {
        commLineStatus = CLS_FREE;
        std::cout << "Error: " << res << std::endl;
        return false;
    }

    callStatus = CS_WAITING;

    int timeout = 15000;
    while (serial.waitForReadyRead(1000)) {
        auto response = serial.readAll();
        if (response.contains("\"SOUNDER\",0")) {
            callStatus = CS_ACTIVE;
            std::cout << "Call is active" << std::endl;
            break;
        }

        if (response.contains("\"SOUNDER\",1")) {
            callStatus = CS_DIALING;
            std::cout << "Call is dialing" << std::endl;
        }

        if (response.contains("NO CARRIER")
        || response.contains("BUSY")
        || response.contains("NO ANSWER")
        || response.contains("ERROR")) {
            callStatus = CS_IDLE;
            std::cout << "Call is idle. Failed to connect" << std::endl;
            break;
        }

        timeout -= 1000;
        if (timeout <= 0) {
            callStatus = CS_IDLE;
            std::cout << "Call is idle. Timeout" << std::endl;
            return false;
        }
    }
    return true;
}

bool Modem::hangUp() {
    if (commLineStatus == CLS_FREE) {
        commLineStatus = CLS_ATCMD;
        Task task(ATH, serial);
        commRes_t res = task.execute();

        if (res == CR_OK) {
            commLineStatus = CLS_FREE;
            callStatus = CS_IDLE;
            return true;
        } else {
            commLineStatus = CLS_FREE;
            return false;
        }
    } else {
        return false;
    }
}

bool Modem::message(const std::string& number, const std::string& message) {
    if (commLineStatus == CLS_FREE) {
        commLineStatus = CLS_ATCMD;
        SetCommand set_message_mode = SetCommand(AT_CMGF"=1", serial);
        set_message_mode.execute();

        SetCommand set_message = SetCommand(AT_CMGS"=\"" + number + "\"", serial);
        set_message.execute();

        Task task(message, serial);
        commRes_t res = task.execute();

        if (res == CR_OK) {
            SetCommand command = SetCommand(message + char(26), serial);
            command.execute();
            commLineStatus = CLS_FREE;
            return true;
        } else {
            commLineStatus = CLS_FREE;
            return false;
        }
    } else {
        return false;
    }
}


bool Modem::initialize() {
    // Check AT
    bool atStatus = checkAT();
    if (!atStatus) {
        qDebug() << "Error: AT command failed";
        return false;
    }

    // Set message mode to TEXT
    SetCommand set_message_mode = SetCommand(AT_CMGF"=1", serial);
    commRes_t messageModeStatus = set_message_mode.execute();

    if (messageModeStatus != CR_OK) {
        qDebug() << "Error: message mode failed";
        return false;
    }

    // Check registration
    bool registrationStatus = checkRegistration();
    if (!registrationStatus) {
        qDebug() << "Error: registration failed";
        return false;
    }

    qDebug() << "Modem initialized";
    return true;
}


void Modem::worker() {
    Modem::workerStatus = true;

    while (Modem::workerStatus) {
        auto output = serial.readAll();
        QThread::msleep(500);
        qDebug() << output;
    }
}

