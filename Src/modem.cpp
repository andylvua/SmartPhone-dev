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
        } else {
            commLineStatus = CLS_FREE;
            return false;
        }
    } else {
        return false;
    }
}