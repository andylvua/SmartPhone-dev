//
// Created by Andrew Yaroshevych on 05.02.2023.
//

#include "logging.hpp"
#include "cli/cli.hpp"
#include "cli/definitions/colors.hpp"
#include "cli/utils/assertions.hpp"
#include "cli/utils/ncurses/ncurses_utils.hpp"
#include "cli/utils/readline/readline_utils.hpp"
#include <readline/readline.h>
#include <readline/history.h>
#include <QRegularExpression>

const auto cliLogger = spdlog::get("cli");
#ifdef BUILD_ON_RASPBERRY
#include "rotary_reader/rotary_dial.hpp"
static RotaryDial rtx;
#endif

void CLI::rejectCall() {
    printColored(YELLOW_PAIR, "Rejecting call");
    modem.hangUp();
    CLI::screenMap["Incoming Call"]->notifications.clear();
    CLI::screenMap["In Call"]->notifications.clear();
    changeScreen("Main");
}

void CLI::answerCall() {
    printColored(YELLOW_PAIR, "Answering call");
    modem.answer();
    changeScreen("In Call");
}

void CLI::call() {
    QString number;
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

    printColored(YELLOW_PAIR, "Calling...");
    modem.call(number);

    CLI::screenMap["In Call"]->addNotification("Calling " + number);
    changeScreen("In Call");
}

void CLI::call(const QString &number) {
    printColored(YELLOW_PAIR, "Calling...");
    modem.call(number);

    CLI::screenMap["In Call"]->addNotification("Calling " + number);
    changeScreen("In Call");
}

void CLI::hangUp() {
    modem.hangUp();
    CLI::screenMap["In Call"]->notifications.clear();

    printColored(YELLOW_PAIR, "Hanging up");
    QThread::msleep(2000);
    changeScreen("Main");
}


void CLI::sendMessage() {
    QString number;
    QString message;
    printColored(YELLOW_PAIR, "Enter number: ");
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

    printColored(YELLOW_PAIR, "Enter message: ");
    message = readString();
    printColored(YELLOW_PAIR, "Sending SMS");

    modem.message(number, message);
}

void CLI::sendMessage(const QString &number) {
    QString message;
    printColored(YELLOW_PAIR, "Enter message: ");
    message = readString();
    printColored(YELLOW_PAIR, "Sending SMS");

    modem.message(number, message);
}

void CLI::setMessageMode() {
    auto messageModeOption = CLI::screenMap["Debug Settings"]->optionsMap["Text Mode"];

    bool success = modem.setMessageMode(!messageModeOption->getState());
    if (success) {
        messageModeOption->switchState();
    } else {
        printColored(RED_PAIR, "Failed to set Text Mode");
    }
    updateScreen();
}

void CLI::setNumberID() {
    auto numberIDOption = CLI::screenMap["Debug Settings"]->optionsMap["Number Identifier"];

    bool success = modem.setNumberID(!numberIDOption->getState());
    if (success) {
        numberIDOption->switchState();
    } else {
        printColored(RED_PAIR, "Failed to set Number Identifier");
    }
    updateScreen();
}

void CLI::setEchoMode() {
    auto echoModeOption = CLI::screenMap["Debug Settings"]->optionsMap["Echo Mode"];

    bool success = modem.setEchoMode(!echoModeOption->getState());
    if (success) {
        echoModeOption->switchState();
    } else {
        printColored(RED_PAIR, "Failed to set Echo Mode");
    }
    updateScreen();
}

void CLI::aboutDevice() {
    changeScreen("About Device");
    printColored(YELLOW_PAIR, "Getting device info", false);
    QString aboutInfo = modem.aboutDevice();
    NcursesUtils::clearCurrentLine();
    printColored(WHITE_PAIR, aboutInfo);
}

void CLI::atConsoleMode() {
    disableNcursesScreen();
    std::cout << YELLOW_COLOR << "Loading..." << RESET;
    std::cout.flush();

    if (!modem.enableATConsoleMode()) {
        std::cout << RED_COLOR << "\nFailed to enable AT console mode" << RESET << std::endl;
        SPDLOG_LOGGER_ERROR(cliLogger, "Failed to enable AT console mode");
        QThread::msleep(1000);
        modem.disableATConsoleMode();

        enableNcursesScreen();
        return;
    }

    initReadlineCompletions();
    rl_attempted_completion_function = commandCompletion;

    const char *at;

    std::cout << "\r" << GREEN_COLOR << "Send AT commands. Type exit to exit:" << RESET << std::endl;

    while ((at = readline("")) != nullptr) {
        if (*at) {
            add_history(at);
        }
        if (strcmp(at, "exit") == 0) {
            break;
        }
        if (strcmp(at, "clear") == 0) {
            system("clear");
            continue;
        }

        modem.sendATConsoleCommand(at);

        free((void *) at);
    }

    modem.disableATConsoleMode();
    enableNcursesScreen();
}

void CLI::ussdConsoleMode() {
    disableNcursesScreen();
    std::cout << YELLOW_COLOR << "Loading..." << RESET;
    std::cout.flush();

    if (!modem.enableUSSDConsoleMode()) {
        std::cout << RED_COLOR << "\nFailed to enable USSD console mode" << RESET << std::endl;
        SPDLOG_LOGGER_ERROR(cliLogger, "Failed to enable USSD console mode");
        QThread::msleep(1000);
        modem.disableUSSDConsoleMode();

        enableNcursesScreen();
        return;
    }

    const char *ussd;

    std::cout << "\r" << GREEN_COLOR << "Send USSD commands. Type 'exit' to exit:" << RESET << std::endl;

    while ((ussd = readline("")) != nullptr) {
        if (*ussd) {
            add_history(ussd);
        }
        if (strcmp(ussd, "exit") == 0) {
            break;
        }
        if (strcmp(ussd, "clear") == 0) {
            system("clear");
            continue;
        }

        modem.sendUSSDConsoleCommand(ussd);

        free((void *) ussd);
    }

    modem.disableUSSDConsoleMode();
    enableNcursesScreen();
}

QString parseUrl(QString httpCommand) {
    QString url;
    url = httpCommand.remove(0, httpCommand.indexOf(' ') + 1);

    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url = "https://" + url;
    }

    url = url.trimmed();

    QRegularExpression urlRegex(
            "^(http|https)://[a-zA-Z0-9-]+(\\.[a-zA-Z0-9-]+)+([a-zA-Z0-9-._~:/?#[\\]@!$&'()*+,;=]*)?$"
    );
    QRegularExpressionMatch match = urlRegex.match(url);

    if (!match.hasMatch()) {
        std::cout << RED_COLOR << "Invalid URL" << RESET << std::endl;
        return "";
    }

    return url;
}

void CLI::httpConsoleMode() {
    disableNcursesScreen();
    std::cout << YELLOW_COLOR << "Loading..." << RESET;
    std::cout.flush();

    auto consoleEnabled = modem.enableHTTPConsoleMode();

    if (!consoleEnabled) {
        std::cout << RED_COLOR << "\nFailed to enable HTTP console mode" << RESET << std::endl;
        SPDLOG_LOGGER_ERROR(cliLogger, "Failed to enable HTTP console mode");
        QThread::msleep(1000);
        modem.disableHTTPConsoleMode();

        enableNcursesScreen();
        return;
    }

    const char *http;

    std::cout << "\r" << GREEN_COLOR
              << "Send HTTP commands. GET and POST are supported. Type exit to exit:" << RESET << std::endl;

    while ((http = readline("")) != nullptr) {
        if (*http) {
            add_history(http);
        }
        if (strcmp(http, "exit") == 0) {
            break;
        }
        if (strcmp(http, "clear") == 0) {
            system("clear");
            continue;
        }

        httpMethod_t method;

        if (strstr(http, "GET ") != nullptr) {
            method = httpMethod::HM_GET;
        } else if (strstr(http, "POST ") != nullptr) {
            method = httpMethod::HM_POST;
        } else {
            std::cout << RED_COLOR << "Invalid HTTP command" << RESET << std::endl;
            continue;
        }

        QString rawCommand = http;
        QString url = parseUrl(rawCommand);

        if (url.isEmpty()) {
            SPDLOG_LOGGER_INFO(cliLogger, "Invalid HTTP command: {}", rawCommand.toStdString());
            continue;
        }

        SPDLOG_LOGGER_INFO(cliLogger, "Sending HTTP command: {}", url.toStdString());
        modem.sendHTTPConsoleCommand(url, method);

        free((void *) http);
    }

    modem.disableHTTPConsoleMode();
    enableNcursesScreen();
}
