//
// Created by paul on 1/14/23.
//

#include <QProcess>
#include "../../Inc/cli/cli.h"
#include "../../Inc/logging.h"
#include "../../Inc/cli/color_print.h"


auto cli_logger = spdlog::basic_logger_mt("cli", "../logs/log.txt", true);

CLI::CLI(Modem &modem, Screen *currentScreen) : modem(modem), currentScreen(currentScreen) {
    connect(&modem, SIGNAL(incomingCall(QString)),
            this, SLOT(handleIncomingCall(QString)));
    connect(&modem, SIGNAL(incomingSMS()), this, SLOT(handleIncomingSMS()));
    connect(&modem, SIGNAL(callEnded()), this, SLOT(handleCallEnded()));
}

void CLI::addScreen(Screen *screen) {
    screens.push_back(screen);
}

void CLI::renderScreen() {
    // Clear the screen
    system("clear");

    //std::cout << currentScreen->screenName.toStdString() << std::endl;
    printColored(BOLDWHITE, currentScreen->screenName.toStdString());

    for (auto notification: currentScreen->notifications) {

        //std::cout << notification.toStdString() << std::endl;
        printColored(WHITE, notification.toStdString());
    }

    for (auto &option: currentScreen->screenOptions) {
        //std::cout << option.toStdString() << std::endl;
        printColored(WHITE, option.toStdString());
    }
}

void CLI::changeScreen(Screen *screen) {
    currentScreen = screen;
}

void CLI::changeScreen(const QString &screenName) {
    for (auto screen: screens) {
        if (screen->screenName == screenName) {
            currentScreen = screen;
        }
    }
}

void CLI::handleIncomingCall(QString number) {
    for (auto screen: screens) {
        if (screen->screenName == "Incoming Call" || screen->screenName == "In Call") {
            screen->addNotification("Incoming call from +" + number);
        }
    }
    changeScreen("Incoming Call");
    renderScreen();
}

void CLI::handleIncomingSMS() {
    qDebug() << "Incoming SMS";

    for (auto screen: screens) {
        if (screen->screenName == "Main" && screen->notifications.empty()) {
            screen->addNotification("    *New SMS");
        }
    }
    renderScreen();
}

void CLI::handleCallEnded() {
    for (auto screen: screens) {
        if (screen->screenName == "In Call" || screen->screenName == "Incoming Call") {
            screen->notifications.clear();
        }
    }
    changeScreen("Main");
    renderScreen();
}

void CLI::mainScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        exit(0);
    }
    if (strcmp(line, "1") == 0) {
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

void CLI::incomingCallScreenHandler(char *line) {
    if (strcmp(line, "1") == 0) {
        // qDebug() << "Answering call";
        printColored(YELLOW, "Answering call");
        modem.answer();
        changeScreen("In Call");
        renderScreen();

    }
    if (strcmp(line, "0") == 0) {
        // qDebug() << "Rejecting call";
        printColored(YELLOW, "Rejecting call");
        modem.hangUp();
        for (auto screen: screens) {
            if (screen->screenName == "Incoming Call" || screen->screenName == "In Call") {
                screen->notifications.clear();
            }
        }
        changeScreen("Main");
        renderScreen();
    }
}

void CLI::phoneScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(currentScreen->parentScreen);
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
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0) {
        changeScreen("Send SMS");
        renderScreen();
    }
    if (strcmp(line, "2") == 0) {
        for (auto screen: screens) {
            if (screen->screenName == "Main") {
                screen->removeNotification("    *New SMS");
            }
        }
        Modem::listMessages();
    }
}

void CLI::ussdScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0) {
        //qDebug() << "In development";
        printColored(BOLDYELLOW, "In development");
//        qDebug() << "Requesting USSD";
//        std::string ussd;
//        qDebug() << "Enter USSD";
//        std::cin >> ussd;
//        modem.sendUSSD(ussd);
//        changeScreen("USSD Console");
//        renderScreen();
    }
}

void CLI::atScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        modem.disableConsoleMode();
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0) {
        modem.enableConsoleMode();
        printColored(GREEN, "AT Console mode enabled. To exit, type 'exit'");
        //std::cout << "AT Console mode enabled. To exit, type 'exit'" << std::endl;
        std::string at{};

        //qDebug() << "Enter AT";
        printColored(WHITE, "Enter commands:");
        while (true) {
            std::cin >> at;
            if (at == "exit") {
                break;
            }
            modem.sendConsoleCommand(QString::fromStdString(at));
        }

        modem.disableConsoleMode();
        renderScreen();
    }
}

void CLI::logsScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0) {
        //qDebug() << "Opening logs";
        printColored(GREEN, "Opening logs");
        system("xdg-open ./../logs/log.txt");
    }
}

void CLI::callScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    }
    if (strcmp(line, "1") == 0) {
        // qDebug() << "Calling";

        std::string number;
        // qDebug() << "Enter number";
        printColored(YELLOW, "Enter number");
        std::cin >> number;
        printColored(YELLOW, "Calling...");
        modem.call(number);
        for (auto screen: screens) {
            if (screen->screenName == "In Call") {
                screen->addNotification("Calling " + QString::fromStdString(number));
            }
        }
        changeScreen("In Call");
        renderScreen();
    }
}

void CLI::inCallScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        modem.hangUp();
        for (auto screen: screens) {
            if (screen->screenName == "Incoming Call" || screen->screenName == "In Call") {
                screen->notifications.clear();
            }
        }
        //qDebug() << "Hanging up";
        printColored(YELLOW, "Hanging up");
        changeScreen("Main");
        renderScreen();
    }
}

void CLI::contactsScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    } else if (strcmp(line, "1") == 0) {
        // qDebug() << "Adding contact";
        printColored(YELLOW, "Adding contact");
        std::string name;
        std::string number;
        // qDebug() << "Enter name";
        printColored(YELLOW, "Enter name");
        std::cin >> name;
        // qDebug() << "Enter number";
        printColored(YELLOW, "Enter number");
        std::cin >> number;
        Modem::addContact(name, number);
        changeScreen("Contacts");
        renderScreen();
    } else if (strcmp(line, "2") == 0) {
        // qDebug() << "Deleting contact";
        printColored(YELLOW, "Deleting contact");
        std::string name;
        // qDebug() << "Enter name";
        printColored(YELLOW, "Enter name");
        std::cin >> name;
        Modem::removeContact(name);
        changeScreen("Contacts");
        renderScreen();
    } else if (strcmp(line, "3") == 0) {
        //qDebug() << "Listing contacts";
        printColored(YELLOW, "Listing contacts");
        Modem::listContacts();
        changeScreen("Contacts");
    }
}

void CLI::sendSMSScreenHandler(char *line) {
    if (strcmp(line, "0") == 0) {
        changeScreen(currentScreen->parentScreen);
        renderScreen();
    } else if (strcmp(line, "1") == 0) {

        std::string number;
        std::string message;
        //qDebug() << "Enter number";
        printColored(YELLOW, "Enter number: ");
        std::cin >> number;
        printColored(YELLOW, "Enter message: ");
        //qDebug() << "Enter message";
        std::getline(std::cin >> std::ws, message);
        //qDebug() << "Sending SMS";
        printColored(YELLOW, "Sending SMS");
        modem.message(number, message);
    }
}

void CLI::listen() {
    cli_logger->flush_on(spdlog::level::debug);
    SPDLOG_LOGGER_INFO(cli_logger, "CLI listener started.");

    char *line;
    renderScreen();
    printColored(BOLDWHITE, ">>> ", false);
    // std::cout << ">>> ";

    while ((line = readline(""))) {
        if (line[0] == '\0') {
            continue;
        }

        if (currentScreen->screenName == "Main") {
            mainScreenHandler(line);
        } else if (currentScreen->screenName == "Incoming Call") {
            incomingCallScreenHandler(line);
        } else if (currentScreen->screenName == "Phone") {
            phoneScreenHandler(line);
        } else if (currentScreen->screenName == "Call") {
            callScreenHandler(line);
        } else if (currentScreen->screenName == "In Call") {
            inCallScreenHandler(line);
        } else if (currentScreen->screenName == "Contacts") {
            contactsScreenHandler(line);
        } else if (currentScreen->screenName == "SMS") {
            smsScreenHandler(line);
        } else if (currentScreen->screenName == "Send SMS") {
            sendSMSScreenHandler(line);
        } else if (currentScreen->screenName == "USSD Console") {
            ussdScreenHandler(line);
        } else if (currentScreen->screenName == "AT Console") {
            atScreenHandler(line);
        } else if (currentScreen->screenName == "Logs") {
            logsScreenHandler(line);
        } else {
            printColored(RED, "Unknown screen");
            //qDebug() << "Unknown screen";
        }
        printColored(BOLDWHITE, ">>> ", false);
      //  std::cout << ">>> ";

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
