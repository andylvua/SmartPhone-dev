//
// Created by Andrew Yaroshevych on 21.12.2022.
//

#include "../../Inc/modem/modem.h"
#include "../../Inc/command/command.h"
#include "../../Inc/command/commands_list.h"
#include "../../Inc/logging.h"
#include "../../Inc/cli/ncurses_io.h"
#include <iostream>
#include <fstream>

const auto modem_logger = spdlog::basic_logger_mt("modem", "../logs/log.txt", true);

Modem::Modem(SerialPort &serial) : serial(serial) {}

void Modem::enableConsoleMode() {
    SPDLOG_LOGGER_INFO(modem_logger, "Console mode enabled");
    serial.write(ATE0"\r\n");
    QThread::msleep(500);
    consoleMode = true;
}

void Modem::disableConsoleMode() {
    SPDLOG_LOGGER_INFO(modem_logger, "Console mode disabled");
    serial.write(ATE1"\r\n");
    consoleMode = false;
}

void Modem::sendConsoleCommand(const QString &command) {
    SPDLOG_LOGGER_INFO(modem_logger, "sendConsoleCommand: {}", command.toStdString());
    serial.write((command.toStdString() + "\r\n").c_str());
}

QString Modem::parseLine(const QByteArray &line) {
    auto lineString = QString(line);
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

void saveMessage(const QString &number, const QString &dateTime, const QString &message, bool incoming) {
    std::string isIncoming = incoming ? "Incoming" : "Outgoing";
    std::string data = number.toStdString() + "; " + isIncoming + "; " + dateTime.toStdString() + "; "
            + message.toStdString();

    if (incoming) {
        data += " * new *";
    }

    SPDLOG_LOGGER_INFO(modem_logger, "Saving message: {}", data);
    writeToFile(MESSAGES_FILEPATH, data);
}

void saveCall(const Call &call) {
    QString dateTime = call.startTime.toString("dd.MM.yyyy hh:mm:ss");
    QString duration = QString::number(call.startTime.secsTo(call.endTime));
    std::string callDirection = call.callDirection == callDirection::CD_INCOMING ? "INCOMING" : "OUTGOING";
    std::string isMissed = call.callResult == callResult::CR_NO_ANSWER ? "missed" : "accepted";

    std::string data = call.number.toStdString() + "; " + dateTime.toStdString() + "; "
                       + duration.toStdString() + "; " + callDirection + "; " + isMissed;
    SPDLOG_LOGGER_INFO(modem_logger, "Saving call: {}", data);
    writeToFile(CALLS_FILEPATH, data);
}

bool Modem::checkAT() {
    auto command = GetCommand(AT, serial);
    auto response = command.execute(false);

    if (response.indexOf("OK") != -1) {
        return true;
    }

    return false;
}

bool Modem::checkRegistration() {
    auto command = GetCommand(AT_CREG"?", serial);
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
    SPDLOG_LOGGER_INFO(modem_logger, "Calling {}", number);
    commRes_t res = task.execute();

    if (res == commRes::CR_OK) {
        SPDLOG_LOGGER_INFO(modem_logger, "Call to {} was successful", number);
        currentCall.callDirection = callDirection::CD_OUTGOING;
        currentCall.callResult = callResult::CR_NO_ANSWER;
        currentCall.number = QString::fromStdString(number);
        currentCall.startTime = QDateTime::currentDateTime();
        return true;
    } else {
        SPDLOG_LOGGER_INFO(modem_logger, "Call to {} failed", number);
        return false;
    }
}

bool Modem::hangUp() {
    Task task(ATH, serial);
    SPDLOG_LOGGER_INFO(modem_logger, "Hanging up with {}", currentCall.number.toStdString());
    commRes_t res = task.execute();

    if (res == commRes::CR_OK) {
        SPDLOG_LOGGER_INFO(modem_logger, "Hanging up was successful");
        return true;
    } else {
        SPDLOG_LOGGER_WARN(modem_logger, "Hanging up failed");
        return false;
    }
}

bool Modem::answer() {
    Task task(ATA, serial);
    SPDLOG_LOGGER_INFO(modem_logger, "Answering {}", currentCall.number.toStdString());
    commRes_t res = task.execute();

    if (res == commRes::CR_OK) {
        SPDLOG_LOGGER_INFO(modem_logger, "Answering was successful");
        return true;
    } else {
        SPDLOG_LOGGER_WARN(modem_logger, "Answering failed");
        return false;
    }
}

bool Modem::message(const std::string &number, const std::string &message) {
    SPDLOG_LOGGER_INFO(modem_logger, "Sending message to {}. Message: {}", number, message);
    auto set_message = GetCommand(AT_CMGS"=\"" + number + "\"", serial);
    auto response = set_message.execute(true, false);

    if (response.indexOf(">") != -1) {
        SPDLOG_LOGGER_INFO(modem_logger, "Received >. Writing message");
        Task task(message + char(26), serial);  // 26 is Ctrl+Z
        commRes_t res = task.execute(false);

        if (res == commRes::CR_OK) {
            printColored(GREEN, "Message sent successfully");
            SPDLOG_LOGGER_INFO(modem_logger, "Message sent");
            saveMessage(QString::fromStdString(number),
                        QDateTime::currentDateTime().toString("yyyy/MM/dd,hh:mm:ss+02"),
                        QString::fromStdString(message),
                        false
            );
            return true;
        }
    } else {
        printColored(RED, "Message sending failed. No > prompt received.");
        SPDLOG_LOGGER_WARN(modem_logger, "Error sending message. No > prompt received.");
        return false;
    }

    return false;
}


bool Modem::initialize() {
    SPDLOG_LOGGER_INFO(modem_logger, "Initializing modem");
    printColored(GREEN, "Performing modem initialization. Please wait", false);
    outStream.flush();

    std::ifstream messagesFile(MESSAGES_FILEPATH);
    SPDLOG_LOGGER_INFO(modem_logger, "Checking messages file...");
    if (!messagesFile.is_open()) {
        std::ofstream createMessagesFile(MESSAGES_FILEPATH);
        createMessagesFile.close();
    }
    SPDLOG_LOGGER_INFO(modem_logger, "Messages file OK");

    std::ifstream callsFile(CALLS_FILEPATH);
    SPDLOG_LOGGER_INFO(modem_logger, "Checking calls file...");
    if (!callsFile.is_open()) {
        std::ofstream createCallsFile(CALLS_FILEPATH);
        createCallsFile.close();
    }
    SPDLOG_LOGGER_INFO(modem_logger, "Calls file OK");

    std::ifstream contactsFile(CONTACTS_FILEPATH);
    SPDLOG_LOGGER_INFO(modem_logger, "Checking contacts file...");
    if (!contactsFile.is_open()) {
        std::ofstream createContactsFile(CONTACTS_FILEPATH);
        createContactsFile.close();
    }
    SPDLOG_LOGGER_INFO(modem_logger, "Contacts file OK");

    SPDLOG_LOGGER_INFO(modem_logger, "Checking AT...");
    bool atStatus = checkAT();
    if (!atStatus) {
        printColored(RED, "AT command failed");
        SPDLOG_LOGGER_ERROR(modem_logger, "AT command failed");
        return false;
    }

    printColored(GREEN, ".", false);
    outStream.flush();

    SPDLOG_LOGGER_INFO(modem_logger, "AT OK");

    SPDLOG_LOGGER_INFO(modem_logger, "Setting SMS mode to text...");
    auto set_message_mode = SetCommand(AT_CMGF"=1", serial);
    commRes_t messageModeStatus = set_message_mode.execute(false);

    if (messageModeStatus != commRes::CR_OK) {
        printColored(RED, "Error setting SMS mode to text");
        SPDLOG_LOGGER_ERROR(modem_logger, "Message mode failed");
        return false;
    }

    printColored(GREEN, ".", false);
    outStream.flush();

    SPDLOG_LOGGER_INFO(modem_logger, "SMS mode OK");

    SPDLOG_LOGGER_INFO(modem_logger, "Checking registration...");
    bool registrationStatus = checkRegistration();
    if (!registrationStatus) {
        printColored(RED, "Error: Registration failed");
        SPDLOG_LOGGER_ERROR(modem_logger, "Registration failed");
        return false;
    }
    SPDLOG_LOGGER_INFO(modem_logger, "Registration OK");

    printColored(GREEN, ".", false);
    outStream.flush();

    SPDLOG_LOGGER_INFO(modem_logger, "Setting number identification...");
    auto setNumberIDTrue = SetCommand("AT+CLIP=1", serial);
    commRes_t numberIdentifierStatus = setNumberIDTrue.execute(false);
    if (numberIdentifierStatus != commRes::CR_OK) {
        printColored(RED, "Error: Number identification failed");
        SPDLOG_LOGGER_ERROR(modem_logger, "Number identification failed");
        return false;
    }
    SPDLOG_LOGGER_INFO(modem_logger, "Number identification OK");

    printColored(GREEN, " ✓", false);
    outStream.flush();

    QThread::msleep(300);
    SPDLOG_LOGGER_INFO(modem_logger, "Modem initialized successfully");
    return true;
}

void Modem::worker() {
    Modem::workerStatus = true;
    SPDLOG_LOGGER_INFO(modem_logger, "Worker started");
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
    SPDLOG_LOGGER_INFO(modem_logger, "Data interrupt handler started");
    serial.buffer.clear();
    if (serial.waitForReadyRead(100)) {
        data = serial.readAll();
        while (serial.waitForReadyRead(100))
            data += serial.readAll();
    }

    serial.buffer = data;
    SPDLOG_LOGGER_INFO(modem_logger, "Buffering data: {}. Exiting data interrupt handler", data.toStdString());
    serial.interruptDataRead = false;
}

void Modem::_ringHandler(const QString &parsedLine) {
    if (parsedLine.contains("CLIP")) {
        QString number = parsedLine.split("\"")[1];
        currentCall = Call{};
        currentCall.callDirection = callDirection::CD_INCOMING;
        currentCall.number = number;
        currentCall.startTime = QDateTime::currentDateTime();
        currentCall.callResult = callResult::CR_NO_ANSWER;
        SPDLOG_LOGGER_INFO(modem_logger, "Incoming call from: {}", number.toStdString());
        emit incomingCall(number);
    }
}

void Modem::_ciev_call_0Handler(const QString &parsedLine) {
    currentCall.endTime = QDateTime::currentDateTime();
    SPDLOG_LOGGER_INFO(modem_logger, "CALL 0 received. Saving call");
    if (parsedLine.contains("BUSY")) {
        currentCall.callResult = callResult::CR_NO_ANSWER;
        printColored(YELLOW, "Call ended: BUSY");
        SPDLOG_LOGGER_INFO(modem_logger, "Call busy");
    }

    saveCall(currentCall);
    SPDLOG_LOGGER_INFO(modem_logger, "Call saved");
    emit callEnded();
    SPDLOG_LOGGER_INFO(modem_logger, "Call ended signal emitted");
    printColored(GREEN, "Call ended");
}


void Modem::_ciev_call_1Handler() {
    if (currentCall.callDirection == callDirection::CD_INCOMING) {
        currentCall.callResult = callResult::CR_ANSWERED;
        printColored(GREEN, "Call answered");
        SPDLOG_LOGGER_INFO(modem_logger, "Call answered");
    }
}

void Modem::_sounder_0Handler() {
    if (currentCall.callDirection == callDirection::CD_OUTGOING) {
        currentCall.callResult = callResult::CR_ANSWERED;
        printColored(YELLOW, "Call answered");
        SPDLOG_LOGGER_INFO(modem_logger, "Call answered/declined");
    }
}

void Modem::_message_1Handler(const QString &parsedLine) {
    if (parsedLine.contains("CMT")) {
        QString number = parsedLine.split("\"")[3];
        QString dateTime = parsedLine.split("\"")[5];
        QString message = parsedLine.split("\"")[6];

        saveMessage(number, dateTime, message, true);
        printColored(YELLOW, ("New message from " + number + ": " + message).toStdString());
        SPDLOG_LOGGER_INFO(modem_logger, "New message from: {} Message: {} Date: {}", number.toStdString(),
                           message.toStdString(), dateTime.toStdString());
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

        if (consoleMode) {
            if (parsedLine.isEmpty()) {
                continue;
            }
            SPDLOG_LOGGER_INFO(modem_logger, "Console mode: {}", parsedLine.toStdString());
            if (parsedLine.contains("ERROR")) {
                printColored(RED, parsedLine.toStdString());
            } else {
                printColored(GREEN, parsedLine.toStdString());
            }
            continue;
        }

        if (parsedLine.contains("RING")) {
            SPDLOG_LOGGER_INFO(modem_logger, "RING received");
            _ringHandler(parsedLine);
        }

        if (parsedLine.contains("CIEV: \"CALL\",1")) {
            SPDLOG_LOGGER_INFO(modem_logger, "CIEV: \"CALL\",1 received");
            _ciev_call_1Handler();
        }

        if (parsedLine.contains("+CIEV: \"SOUNDER\",0")) {
            SPDLOG_LOGGER_INFO(modem_logger, "+CIEV: \"SOUNDER\",0 received");
            _sounder_0Handler();
        }

        if (parsedLine.contains("CIEV: \"CALL\",0")) {
            SPDLOG_LOGGER_INFO(modem_logger, "CIEV: \"CALL\",0 received");
            _ciev_call_0Handler(parsedLine);
        }

        if (parsedLine.contains("CIEV: \"MESSAGE\",1")) {
            SPDLOG_LOGGER_INFO(modem_logger, "CIEV: \"MESSAGE\",1 received");
            _message_1Handler(parsedLine);
        }
    }
}


void Modem::addContact(const std::string &name, const std::string &number) {
    std::string data = name + "; " + number;
    SPDLOG_LOGGER_INFO(modem_logger, "Adding contact: {}", data);
    writeToFile(CONTACTS_FILEPATH, data);
    SPDLOG_LOGGER_INFO(modem_logger, "Contact added");
}

void Modem::removeContact(const std::string &name) {
    SPDLOG_LOGGER_INFO(modem_logger, "Removing contact: {}", name);
    std::ifstream file(CONTACTS_FILEPATH);
    std::string line;
    std::string data;

    while (std::getline(file, line)) {
        if (line.find(name) == std::string::npos)
            data += line + "\n";
    }

    file.close();

    std::ofstream contactsFile(CONTACTS_FILEPATH);
    contactsFile << data;
    SPDLOG_LOGGER_INFO(modem_logger, "Contact removed");
}

void Modem::listContacts() {
    SPDLOG_LOGGER_INFO(modem_logger, "Listing contacts");
    std::ifstream file(CONTACTS_FILEPATH);
    std::string line;
    std::string data;

    while (std::getline(file, line)) {
        auto contact = QString::fromStdString(line).split("; ");
        data += std::string("Name: " + contact[0].toStdString() + "\n" + " Number: " + contact[1].toStdString() + "\n");
    }

    displayPad(data, "Viewing contacts");

    file.close();
}

void removeNewMessageNotification() {
    std::ifstream file(MESSAGES_FILEPATH);
    std::string line;
    std::string data;

    while (std::getline(file, line)) {
        if (line.find("* new *") == std::string::npos) {
            data += line + "\n";
        } else {
            data += line.replace(line.find("* new *"), 7, "") + "\n";
        }
    }

    file.close();

    std::ofstream messagesFile(MESSAGES_FILEPATH);
    messagesFile << data;
}

void Modem::listMessages() {
    SPDLOG_LOGGER_INFO(modem_logger, "Listing messages");
    std::ifstream file(MESSAGES_FILEPATH);
    std::string line;
    std::string data;

    while (std::getline(file, line)) {
        auto message = QString::fromStdString(line).split("; ");
        data += std::string("Number: " + message[0].toStdString() + "\n"
                  + " Direction: " + message[1].toStdString() + "\n"
                  + " Date: " + message[2].toStdString() + "\n"
                  + " Message: " + message[3].toStdString()
                  + "\n");
    }

    displayPad(data, "Viewing messages");

    removeNewMessageNotification();
}

void Modem::listCalls() {
    SPDLOG_LOGGER_INFO(modem_logger, "Listing calls");
    std::ifstream file(CALLS_FILEPATH);
    std::string line;
    std::string data;

    while (std::getline(file, line)) {
        auto call = QString::fromStdString(line).split("; ");
        data += std::string("Number: " + call[0].toStdString() + "\n"
                  + " Date: " + call[1].toStdString() + "\n"
                  + " Duration: " + call[2].toStdString() + "\n"
                  + " Direction: " + call[3].toStdString() + "\n"
                  + " Call result: " + call[4].toStdString()
                  + "\n");
    }

    SPDLOG_LOGGER_INFO(modem_logger, data);

    displayPad(data, "Viewing calls");

    file.close();
}
