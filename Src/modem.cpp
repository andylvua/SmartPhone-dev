//
// Created by Andrew Yaroshevych on 21.12.2022.
//

#include "../Inc/modem.h"
#include "../Inc/command.h"
#include "../Inc/commands_list.h"
#include <iostream>
#include <fstream>

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

QString Modem::parseLine(const QByteArray &line) {
    QString lineString = QString(line);
    if (lineString.isEmpty())
        return lineString;
    QString parsedLine = lineString.replace("\r\n\r\n", "\n").replace("\r\n", "");
    return parsedLine;
}

bool Modem::checkAT() {
    if (commLineStatus != CLS_FREE) {
        return false;
    }

    commLineStatus = CLS_ATCMD;
    GetCommand command = GetCommand(AT, serial);
    auto response = command.execute();
    commLineStatus = CLS_FREE;

    if (response.indexOf("OK") != -1) {
        return true;
    }

    return false;
}

bool Modem::checkRegistration() {
    if (commLineStatus != CLS_FREE) {
        return false;
    }

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
}

bool Modem::call(const std::string &number) {
    if (commLineStatus != CLS_FREE) {
        return false;
    }

    commLineStatus = CLS_ATCMD;
    Task task(ATD + number + ";", serial);
    commRes_t res = task.execute();
    commLineStatus = CLS_FREE;

    if (res == CR_OK) {
        currentCall.callDirection = CD_OUTGOING;
        currentCall.callResult = CR_NO_ANSWER;
        currentCall.number = QString::fromStdString(number);
        currentCall.startTime = QDateTime::currentDateTime();
        callStatus = CS_DIALING;
        return true;
    }

    return false;
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

bool Modem::answer() {
    if (commLineStatus == CLS_FREE) {
        commLineStatus = CLS_ATCMD;
        Task task(ATA, serial);
        commRes_t res = task.execute();

        if (res == CR_OK) {
            commLineStatus = CLS_FREE;
            callStatus = CS_ACTIVE;
            return true;
        } else {
            commLineStatus = CLS_FREE;
            return false;
        }
    } else {
        return false;
    }
}

bool Modem::message(const std::string &number, const std::string &message) {
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
    std::ifstream messagesFile("messages.txt");

    if (!messagesFile.is_open()) {
        std::ofstream createMessagesFile("messages.txt");
        createMessagesFile.close();
    }

    std::ifstream callsFile("calls.txt");

    if (!callsFile.is_open()) {
        std::ofstream createCallsFile("calls.txt");
        createCallsFile.close();
    }

    std::ifstream contactsFile("contacts.txt");

    if (!contactsFile.is_open()) {
        std::ofstream createContactsFile("contacts.txt");
        createContactsFile.close();
    }

    bool atStatus = checkAT();
    if (!atStatus) {
        qDebug() << "Error: AT command failed";
        return false;
    }

    SetCommand set_message_mode = SetCommand(AT_CMGF"=1", serial);
    commRes_t messageModeStatus = set_message_mode.execute();

    if (messageModeStatus != CR_OK) {
        qDebug() << "Error: message mode failed";
        return false;
    }

    bool registrationStatus = checkRegistration();
    if (!registrationStatus) {
        qDebug() << "Error: registration failed";
        return false;
    }

    SetCommand setNumberIDTrue = SetCommand("AT+CLIP=1", serial);
    commRes_t numberIdentifierStatus = setNumberIDTrue.execute();
    if (numberIdentifierStatus != CR_OK) {
        qDebug() << "Error: number identification failed";
        return false;
    }

    qDebug() << "Modem initialized";
    return true;
}

void writeToFile(const std::string &fileName, const std::string &data) {
    std::ofstream file(fileName, std::ios::app);
    file << data << std::endl;
    file.close();
}

void saveMessage(const QString &number, const QString &dateTime, const QString &message) {
    std::string data = number.toStdString() + "; " + dateTime.toStdString() + "; " + message.toStdString();
    writeToFile("messages.txt", data);
}

void saveCall(const Call &call) {
    QString dateTime = call.startTime.toString("dd.MM.yyyy hh:mm:ss");
    QString duration = QString::number(call.startTime.secsTo(call.endTime));
    std::string callDirection = call.callDirection == CD_INCOMING ? "INCOMING" : "OUTGOING";
    std::string isMissed = call.callResult == CR_NO_ANSWER ? "missed" : "accepted";

    std::string data = call.number.toStdString() + "; " + dateTime.toStdString() + "; "
                       + duration.toStdString() + "; " + callDirection + "; " + isMissed;
    writeToFile("calls.txt", data);
}

void Modem::worker() {
    Modem::workerStatus = true;
    Modem::listen();
}

QByteArray Modem::readLine() {
    QByteArray data;

    while (serial.waitForReadyRead(100)) {
        if (serial.bytesAvailable())
            data += serial.readAll();
    }

    return data;
}

void Modem::dataInterruptHandler() {
    QByteArray data;

    serial.buffer.clear();
    if (serial.waitForReadyRead(serial.timeout)) {
        data = serial.readAll();
        while (serial.waitForReadyRead(serial.timeout))
            data += serial.readAll();
    }

    serial.buffer = data;
    serial.interruptDataRead = false;
}

void Modem::_ringHandler(const QString &parsedLine) {
    if (parsedLine.contains("CLIP")) {
        QString number = parsedLine.split("\"")[1];
        currentCall = Call{};
        currentCall.callDirection = CD_INCOMING;
        currentCall.number = number;
        currentCall.startTime = QDateTime::currentDateTime();
        currentCall.callResult = CR_NO_ANSWER;
        qDebug() << "Incoming call from: " << number;
        callStatus = CS_INCOMING;
        emit incomingCall();
    }
}

void Modem::_ciev_call_0Handler(const QString &parsedLine) {
    currentCall.endTime = QDateTime::currentDateTime();

    if (parsedLine.contains("BUSY")) {
        currentCall.callResult = CR_NO_ANSWER;
        qDebug() << "Call busy";
    }

    callStatus = CS_IDLE;
    saveCall(currentCall);

    qDebug() << "Call ended";
}


void Modem::_ciev_call_1Handler() {
    if (currentCall.callDirection == CD_INCOMING) {
        currentCall.callResult = CR_ANSWERED;
        callStatus = CS_ACTIVE;
        qDebug() << "Call answered";
    }

    qDebug() << "Outgoing call";
}

void Modem::_sounder_0Handler() {
    if (currentCall.callDirection == CD_OUTGOING) {
        currentCall.callResult = CR_ANSWERED;
        callStatus = CS_ACTIVE;
        qDebug() << "Call answered/declined";
    }
}

void Modem::_message_1Handler(const QString &parsedLine) {
    if (parsedLine.contains("CMT")) {
        QString number = parsedLine.split("\"")[3];
        QString dateTime = parsedLine.split("\"")[5];
        QString message = parsedLine.split("\"")[6];

        saveMessage(number, dateTime, message);
        qDebug() << "New message from: " << number << " Message: " << message << " Date: " << dateTime;
        emit incomingSMS();
    }
}

void Modem::listen() {
    while (Modem::workerStatus) {
        if (serial.interruptDataRead) {
            dataInterruptHandler();
        }

        QByteArray data = readLine();
        QString parsedLine = parseLine(data);

        if (parsedLine.contains("RING")) {
            _ringHandler(parsedLine);
        }

        if (parsedLine.contains("CIEV: \"CALL\",1")) {
            _ciev_call_1Handler();
        }

        if (parsedLine.contains("+CIEV: \"SOUNDER\",0")) {
            _sounder_0Handler();
        }

        if (parsedLine.contains("CIEV: \"CALL\",0")) {
            _ciev_call_0Handler(parsedLine);
        }

        if (parsedLine.contains("CIEV: \"MESSAGE\",1")) {
            _message_1Handler(parsedLine);
        }
    }
}

void Modem::performCall(const QString &number) {
    qDebug() << "Performing call: " << number;
    call(number.toStdString());
}