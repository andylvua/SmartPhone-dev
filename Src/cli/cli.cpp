//
// Created by paul on 1/14/23.
//

#include "logging.hpp"
#include "cli/cli.hpp"
#include "cli/utils/ncurses/ncurses_utils.hpp"
#include "cli/definitions/colors.hpp"
#include "modem/utils/cache_manager.hpp"
#include "cli/utils/assertions.hpp"
#include <QProcess>
#include <string>

const auto cliLogger = spdlog::basic_logger_mt("cli",
                                               LOGS_FILEPATH, true);
#ifdef BUILD_ON_RASPBERRY
#include "rotary_reader/rotary_dial.hpp"
static RotaryDial rtx;
#endif

CLI::CLI(Modem &modem) : modem(modem) {
    prepareScreens();

#ifdef BUILD_ON_RASPBERRY
#include "rotary_reader/rotary_dial.hpp"
    RotaryDial::setup();
#endif

    connect(&modem, SIGNAL(incomingCall(QString)),
            this, SLOT(handleIncomingCall(QString)));
    connect(&modem, SIGNAL(incomingSMS()), this, SLOT(handleIncomingSMS()));
    connect(&modem, SIGNAL(callEnded()), this, SLOT(handleCallEnded()));
}


void CLI::listen() const {
    cliLogger->flush_on(spdlog::level::debug);
    SPDLOG_LOGGER_INFO(cliLogger, "CLI listener started.");

    NcursesUtils::initScreen();
    renderScreen();

    int ch;

    while (true) {
        if ((ch = getch()) == 27) { // ESC
            NcursesUtils::releaseScreen();
            exit(0);
        }

        switch (ch) {
            case KEY_UP:
                CLI::decrementActiveOption();
                break;
            case KEY_DOWN:
                CLI::incrementActiveOption();
                break;
            case KEY_LEFT:
                CLI::decrementActivePage();
                break;
            case KEY_RIGHT:
                CLI::incrementActivePage();
                break;
            case '\n':
                if (currentScreen->activeOption == -1) {
                    break;
                }
                currentScreen->screenOptions[currentScreen->getActiveOption()]->execute();
                break;
            default:
                break;
        }
    }
}

void CLI::handleIncomingCall(const QString &number) {
    auto contact = CacheManager::getContact(number);

    QString info = "Incoming call from ";

    if (contact.hasValue()) {
        info += contact.name;
    } else {
        info += number;
    }

    CLI::screenMap["Incoming Call"]->addNotification(info);
    CLI::screenMap["In Call"]->addNotification(info);
    changeScreen("Incoming Call");
}

void CLI::handleIncomingSMS() {
    CLI::screenMap["Main"]->addNotification("    *New SMS");
    renderScreen();
}

void CLI::handleCallEnded() {
    CLI::screenMap["Incoming Call"]->notifications.clear();
    CLI::screenMap["In Call"]->notifications.clear();
    changeScreen("Main");
}

void CLI::viewCallHistory() const {
    printColored(YELLOW_PAIR, "Call history:");
    CacheManager::listCalls();
    renderScreen();
}

void CLI::addContact() {
    printColored(YELLOW_PAIR, "Adding contact");
    QString name;
    QString number;
    printColored(YELLOW_PAIR, "Enter name");
    name = readString();
    printColored(YELLOW_PAIR, "Enter number");
#ifdef BUILD_ON_RASPBERRY
    printColored(YELLOW_PAIR, "Read from rotary dial or keyboard? (r/k)");
    QString input;
    input = readString();

    if (input == "r") {
        printColored(YELLOW_PAIR, "Reading from rotary dial");
        number = rtx.listen_for_number();
    } else if (input == "k") {
        printColored(YELLOW_PAIR, "Reading from keyboard");
        number = readString();
    } else {
        printColored(RED_PAIR, "Invalid input");
        return;
    }
#else
    number = readString();
#endif
    if (!assertNumberCorrectness(number)) {
        return;
    }

    CacheManager::addContact(name, number);
    printColored(GREEN_PAIR, "Contact added. Press any key to continue");
    getch();
    changeScreen("Contacts");
}

void CLI::viewContacts() {
    QVector<Contact> contacts = CacheManager::getContacts();
    auto contactsPage = CLI::screenMap["Contacts Page"];

    contactsPage->screenOptions.erase(
            contactsPage->screenOptions.begin() + 1,
            contactsPage->screenOptions.end());

    for (const auto &contact: contacts) {
        contactsPage->addScreenOption(
                contact.name + ": " + contact.number,
                [contact, this]() {
                    auto contactScreen = QSharedPointer<ContactScreen>::create(
                            CLI::screenMap["Contacts Page"],
                            contact,
                            *this);

                    currentScreen = contactScreen;
                    renderScreen();
                });
    }
    changeScreen("Contacts Page");
    renderScreen();
}

void CLI::viewMessages() {
    CLI::screenMap["Main"]->notifications.clear();
    printColored(YELLOW_PAIR, "Listing messages");
    CacheManager::listMessages();
    renderScreen();
}


void CLI::viewLogs() const {
    printColored(GREEN_PAIR, "Opening logs file");
    CacheManager::listLogs();
    renderScreen();
}
