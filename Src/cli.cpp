//
// Created by paul on 1/14/23.
//

#include <QProcess>
#include "../Inc/cli.h"

CLI::CLI(Modem &modem, Screen currentScreen) : modem(modem), currentScreen(currentScreen) {
    connect(&modem, SIGNAL(incomingCall()), this, SLOT(handleIncomingCall()));
    connect(&modem, SIGNAL(incomingSMS()), this, SLOT(handleIncomingSMS()));
    connect(&modem, SIGNAL(callEnded()), this, SLOT(handleCallEnded()));
}
void CLI::addScreen(Screen &screen) {
    screens.push_back(screen);
}
void CLI::renderScreen() {
    // Clear the screen
//    std::cout << "\033[2J\033[1;1H";
    system("clear");

    std::cout << currentScreen.screenName.toStdString() << std::endl;
    for (auto &option : currentScreen.screenOptions) {
        std::cout << option.toStdString() << std::endl;
    }
}
void CLI::changeScreen(Screen &screen) {
    currentScreen = screen;
}

void CLI::changeScreen(const QString &screenName) {
    for (auto &screen: screens) {
        if (screen.screenName == screenName) {
            currentScreen = screen;
        }
    }
}
void CLI::handleIncomingCall()  {
    changeScreen("Incoming Call");
    renderScreen();
}

void CLI::handleIncomingSMS() {
    qDebug() << "Incoming SMS";
    modem.checkAT();
}
void CLI::handleCallEnded() {
    qDebug() << "Call Ended";
    changeScreen("Main");
    renderScreen();
}
void CLI::mainScreenHandler(char* line) {
    if (strcmp(line, "1") == 0) {
        //modem.shutdown();
        changeScreen("Phone");
        renderScreen();
    }
    if (strcmp(line, "2") == 0) {
        changeScreen("SMS");
        renderScreen();
    }
    if (strcmp(line, "3") == 0) {
        changeScreen("USSD Console");
        renderScreen();
    }
    if (strcmp(line, "4") == 0) {
        changeScreen("AT Console");
        renderScreen();
    }
    if (strcmp(line, "5") == 0) {
        changeScreen("Logs");
        renderScreen();
    }
}
void CLI::incomingCallScreenHandler(char* line) {
    if (strcmp(line, "1") == 0) {
        qDebug() << "Answering call";
        modem.answer();
        changeScreen("In Call");
        renderScreen();

    }
    if (strcmp(line, "0") == 0) {
        qDebug() << "Rejecting call";
        modem.hangUp();
        changeScreen("Main");
        renderScreen();
    }
}
void CLI::phoneScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(*currentScreen.parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0) {

        changeScreen("Call");
        renderScreen();
    }
    if (strcmp(line, "2") == 0) {
        changeScreen("Contacts");
        renderScreen();
    }
}
void CLI::smsScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(*currentScreen.parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0) {
        changeScreen("Send SMS");
        renderScreen();
    }
    if (strcmp(line, "2") == 0) {
        Modem::listMessages();
        renderScreen();
    }
}
void CLI::ussdScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(*currentScreen.parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0) {
        qDebug() << "Requesting USSD";
        std::string ussd;
        qDebug() << "Enter USSD";
        std::cin >> ussd;
//        modem.sendUSSD(ussd);
        changeScreen("USSD Console");
        renderScreen();
    }
}
void CLI::atScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(*currentScreen.parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0) {
        qDebug() << "Requesting AT";
        std::string at;
        qDebug() << "Enter AT";
        std::cin >> at;
//        modem.sendAT(at);
        changeScreen("AT Console");
        renderScreen();
    }
}
void CLI::logsScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(*currentScreen.parentScreen);
        renderScreen();
    }
}
void CLI::callScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(*currentScreen.parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0) {
        qDebug() << "Calling";
        std::string number;
        qDebug() << "Enter number";
        std::cin >> number;
        modem.call(number);
        changeScreen("In Call");
        renderScreen();
    }
}
void CLI::inCallScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        modem.hangUp();
        qDebug() << "Hanging up";
        changeScreen("Main");
        renderScreen();
    }
}
void CLI::contactsScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(*currentScreen.parentScreen);
        renderScreen();
    }
    else if (strcmp(line, "1") == 0) {
        qDebug() << "Adding contact";
        std::string name;
        std::string number;
        qDebug() << "Enter name";
        std::cin >> name;
        qDebug() << "Enter number";
        std::cin >> number;
        Modem::addContact(name, number);
        changeScreen("Contacts");
        renderScreen();
    }
    else if (strcmp(line, "2") == 0) {
        qDebug() << "Deleting contact";
        std::string name;
        qDebug() << "Enter name";
        std::cin >> name;
        Modem::removeContact(name);
        changeScreen("Contacts");
        renderScreen();
    }
    else if (strcmp(line, "3") == 0) {
        qDebug() << "Listing contacts";
        Modem::listContacts();
        changeScreen("Contacts");
//        renderScreen();
    }
}
void CLI::sendSMSScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(*currentScreen.parentScreen);
        renderScreen();
    } else if (strcmp(line, "1") == 0) {

        std::string number;
        std::string message;
        qDebug() << "Enter number";
        std::cin >> number;
        qDebug() << "Enter message";
        std::cin >> message;
        modem.message(number, message);
        qDebug() << "Sending SMS";
    }
}

void CLI::listen() {
    char *line;
    renderScreen();
    std::cout << ">>> ";

    while ((line = readline(""))) {
        if (line[0] == '\0') {
            continue;
        }

        if (currentScreen.screenName == "Main") {
            mainScreenHandler(line);
        } else if (currentScreen.screenName == "Incoming Call") {
            incomingCallScreenHandler(line);
        } else if (currentScreen.screenName == "Phone") {
            phoneScreenHandler(line);
        } else if (currentScreen.screenName == "Call") {
            callScreenHandler(line);
        }
        else if (currentScreen.screenName == "In Call"){
            inCallScreenHandler(line);
        }
        else if (currentScreen.screenName == "Contacts"){
            contactsScreenHandler(line);
        }
        else if (currentScreen.screenName == "SMS"){
            smsScreenHandler(line);
        }
        else if (currentScreen.screenName == "Send SMS"){
            sendSMSScreenHandler(line);
        }
        else if (currentScreen.screenName == "USSD Console"){
            ussdScreenHandler(line);
        }
        else if (currentScreen.screenName == "AT Console"){
            atScreenHandler(line);
        }
        else if (currentScreen.screenName == "Logs"){
            logsScreenHandler(line);
        }
        else if (currentScreen.screenName == "SMS") {
            smsScreenHandler(line);
        } else if (currentScreen.screenName == "USSD Console") {
            ussdScreenHandler(line);
        } else if (currentScreen.screenName == "AT Console") {
            atScreenHandler(line);
        } else if (currentScreen.screenName == "Logs") {
            logsScreenHandler(line);
        } else if (currentScreen.screenName == "Send SMS") {
            sendSMSScreenHandler(line);
        } else {
            qDebug() << "Unknown screen";
        }

        std::cout << ">>> ";

        //        if ((strcmp(line, "exit") == 0) || (strcmp(line, "quit") == 0)|| (strcmp(line, "0") == 0)) {
        //            if (currentScreen.parentScreen != nullptr) {
        //                changeScreen(*currentScreen.parentScreen);
        //                renderScreen();
        //            }
        //        }
        //        if ((strcmp(line, "1") == 0)||(strcmp(line, "Phone") == 0)) {
        //            changeScreen("Phone");
        //            renderScreen();
        //        }
        //
        //        if (strcmp(line, "call") == 0) {
        //            qDebug() << "Requesting call";
        //            modem.call("+380679027620");
        //        }
    }
}
