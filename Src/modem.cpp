//
// Created by Andrew Yaroshevych on 21.12.2022.
//

#include "../Inc/modem.h"
#include "../Inc/command.h"
#include "../Inc/commands_list.h"
#include <iostream>
#include <fstream>

Modem::Modem(SerialPort &serial) : serial(serial) {
    callStatus = CS_IDLE;
}

QString Modem::parseLine(const QByteArray &line) {
    QString lineString = QString(line);
    if (lineString.isEmpty())
        return lineString;
    QString parsedLine = lineString.replace("\r\n\r\n", "\n").replace("\r\n", "");
    return parsedLine;
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

bool Modem::checkAT() {
    GetCommand command = GetCommand(AT, serial);
    auto response = command.execute(false);

    if (response.indexOf("OK") != -1) {
        return true;
    }

    return false;
}

bool Modem::checkRegistration() {
    GetCommand command = GetCommand(AT_CREG"?", serial);
    auto response = command.execute(false);

    if (response.indexOf("+CREG: 0,1") != -1
        || (response.indexOf("+CREG: 0,5") != -1)
        || (response.indexOf("+CREG: 1,1") != -1)
        || (response.indexOf("+CREG: 1,5") != -1)) {
        return true;
    }

    return false;
}

bool Modem::call(const std::string &number) {
    Task task(ATD + number + ";", serial);
    commRes_t res = task.execute();

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

    Task task(ATH, serial);
    commRes_t res = task.execute();

    if (res == CR_OK) {
        callStatus = CS_IDLE;
        return true;
    } else {
        return false;
    }
}

bool Modem::answer() {
    Task task(ATA, serial);
    commRes_t res = task.execute();

    if (res == CR_OK) {
        callStatus = CS_ACTIVE;
        return true;
    } else {
        return false;
    }
}

bool Modem::message(const std::string &number, const std::string &message) {
    GetCommand set_message = GetCommand(AT_CMGS"=\"" + number + "\"", serial);
    auto response = set_message.execute(true, false);

    if (response.indexOf(">") != -1) {
        Task task(message + char(26), serial);  // 26 is Ctrl+Z
        commRes_t res = task.execute(false);

        if (res == CR_OK) {
            qDebug() << "Message sent";
            saveMessage(QString::fromStdString(number),
                        QDateTime::currentDateTime().toString("yyyy/MM/dd,hh:mm:ss+02"),
                        QString::fromStdString(message)
                        );
            return true;
        }
    } else {
        qDebug() << "Error sending message. No > prompt received.";
        return false;
    }
    qDebug() << response;
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
    if (serial.waitForReadyRead(100)) {
        data = serial.readAll();
        while (serial.waitForReadyRead(100))
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
        emit incomingCall(number);
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
    emit callEnded();
    qDebug() << "Call ended";
}


void Modem::_ciev_call_1Handler() {
    if (currentCall.callDirection == CD_INCOMING) {
        currentCall.callResult = CR_ANSWERED;
        callStatus = CS_ACTIVE;
        qDebug() << "Call answered";
    }
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


void Modem::addContact(const std::string &name, const std::string &number) {
    std::string data = name + "; " + number;
    writeToFile("contacts.txt", data);
}

void Modem::removeContact(const std::string &name) {
    std::ifstream file("contacts.txt");
    std::string line;
    std::string data;

    while (std::getline(file, line)) {
        if (line.find(name) == std::string::npos)
            data += line + "\n";
    }

    file.close();

    std::ofstream contactsFile("contacts.txt");
    contactsFile << data;
}

void Modem::listContacts() {
    std::ifstream file("contacts.txt");
    std::string line;
    std::string data;

    while (std::getline(file, line)) {
        auto contact = QString::fromStdString(line).split("; ");
        qDebug() << "Name: " << contact[0] << "\n" << " Number: " << contact[1] << "\n";
    }
}
void Modem::listMessages() {
    std::ifstream file("messages.txt");
    std::string line;
    std::string data;

    while (std::getline(file, line)) {
        auto message = QString::fromStdString(line).split("; ");
        qDebug() << "Number: " << message[0] << "\n" << " Date: " << message[1] << "\n" << " Message: " << message[2] << "\n";
    }
}