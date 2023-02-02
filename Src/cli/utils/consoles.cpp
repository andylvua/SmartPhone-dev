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
#include <regex>

const auto cliLogger = spdlog::get("cli");

void CLI::atConsoleMode() {
    curs_set(1);
    NcursesUtils::releaseScreen();
    system("clear");

    std::cout << YELLOW_COLOR << "Loading..." << RESET;
    std::cout.flush();

    modem.enableATConsoleMode();

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

    NcursesUtils::initScreen();
    renderScreen();
}

void CLI::ussdConsoleMode() {
    curs_set(1);
    NcursesUtils::releaseScreen();
    system("clear");

    std::cout << YELLOW_COLOR << "Loading..." << RESET;
    std::cout.flush();

    modem.enableUSSDConsoleMode();

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

    NcursesUtils::initScreen();
    renderScreen();
}

std::string parseUrl(std::string httpCommand) {
    std::string url;
    url = httpCommand.erase(0, httpCommand.find(' ') + 1);

    if (!url.starts_with("http://") && !url.starts_with("https://")) {
        url = "https://" + url;
    }

    url.erase(0, url.find_first_not_of(' '));
    url.erase(url.find_last_not_of(' ') + 1);

    std::basic_regex<char> urlRegex(
            "^(http|https)://[a-zA-Z0-9-]+(\\.[a-zA-Z0-9-]+)+([a-zA-Z0-9-._~:/?#[\\]@!$&'()*+,;=]*)?$"
    );
    if (!std::regex_match(url, urlRegex)) {
        std::cout << RED_COLOR << "Invalid URL" << RESET << std::endl;
        return "";
    }

    return url;
}

void CLI::httpConsoleMode() {
    curs_set(1);
    NcursesUtils::releaseScreen();
    system("clear");

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

        std::string rawCommand = http;
        std::string url = parseUrl(rawCommand);

        if (url.empty()) {
            SPDLOG_LOGGER_INFO(cliLogger, "Invalid HTTP command: {}", rawCommand);
            continue;
        }

        SPDLOG_LOGGER_INFO(cliLogger, "Sending HTTP command: {}", url);
        modem.sendHTTPConsoleCommand(QString::fromStdString(url), method);

        free((void *) http);
    }

    modem.disableHTTPConsoleMode();

    NcursesUtils::initScreen();
    renderScreen();
}
