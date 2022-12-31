//
// Created by Andrew Yaroshevych on 21.12.2022.
//

#include "../Inc/modem.h"
#include "../Inc/command.h"
#include "../Inc/commands_list.h"

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

        return response == "OK";
    } else {
        return false;
    }
}

bool Modem::call(const std::string& number) {
    if (commLineStatus == CLS_FREE) {
        commLineStatus = CLS_ATCMD;
        Task task(ATD + number + ";", serial);
        commRes_t res = task.execute();

        if (res == CR_OK) {
            commLineStatus = CLS_FREE;
            callStatus = CS_DIALING;

            int timeout = 15000;
            while (serial.waitForReadyRead(1000)) {
                auto response = serial.readAll();
                // TODO: check what message modem send when call is accepted: {call_accepted_message}
                if (response.contains("{call_accepted_message}")) {
                    callStatus = CS_ACTIVE;
                    break;
                } else if (response.contains("NO CARRIER")) {
                    callStatus = CS_IDLE;
                    break;
                }
                timeout -= 1000;
                if (timeout <= 0) {
                    callStatus = CS_IDLE;
                    return false;
                }
            }
            return true;
        } else {
            commLineStatus = CLS_FREE;
            return false;
        }
    } else {
        return false;
    }
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