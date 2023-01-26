//
// Created by Andrew Yaroshevych on 17.01.2023.
//

#include "../../Inc/cli/ncurses_io.hpp"
#include "../../Inc/cli/colors.hpp"
#include <algorithm>

void ColorPrint::initColors() {
    start_color();
    init_pair(WHITE_PAIR, COLOR_WHITE, COLOR_BLACK);
    init_pair(RED_PAIR, COLOR_RED, COLOR_BLACK);
    init_pair(GREEN_PAIR, COLOR_GREEN, COLOR_BLACK);
    init_pair(YELLOW_PAIR, COLOR_YELLOW, COLOR_BLACK);
    init_pair(FILLED_WHITE_PAIR, COLOR_BLACK, COLOR_WHITE);
}

void printColored(int color, const std::string &text, bool newLine, bool bold) {
    attron(COLOR_PAIR(color));
    if (bold) {
        attron(A_BOLD);
    }

    if (newLine) {
        printw("%s\n", text.c_str());
        refresh();
    } else {
        printw("%s", text.c_str());
        refresh();
    }

    attroff(COLOR_PAIR(color));
    attroff(A_BOLD);
}

void printColored(int color, const std::string &text, bool newLine, bool bold, WINDOW *window) {
    wattron(window, COLOR_PAIR(color));
    if (bold) {
        wattron(window, A_BOLD);
    }

    if (newLine) {
        wprintw(window, "%s\n", text.c_str());
        wrefresh(window);
    } else {
        wprintw(window, "%s", text.c_str());
        wrefresh(window);
    }

    wattroff(window, COLOR_PAIR(color));
    wattroff(window, A_BOLD);
}

std::string readString(size_t bufferSize) {
    echo();
    char buffer[bufferSize];
    getstr(buffer);
    noecho();
    return {buffer};
}

std::string readString(size_t bufferSize, WINDOW *window) {
    echo();
    char buffer[bufferSize];
    wgetstr(window, buffer);
    noecho();
    return {buffer};
}
