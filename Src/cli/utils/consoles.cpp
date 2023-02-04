//
// Created by Andrew Yaroshevych on 01.02.2023.
//

#include "cli/cli.hpp"
#include "cli/utils/ncurses_utils.hpp"
#include "cli/definitions/colors.hpp"
#include "logging.hpp"
#include "cli/utils/io/readline_utils.hpp"
#include <readline/readline.h>
#include <readline/history.h>
#include <QRegularExpression>

const auto cliLogger = spdlog::get("cli");

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

        disableNcursesScreen();
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
        modem.disableHTTPConsoleMode();

        NcursesUtils::initScreen();
        renderScreen();
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
