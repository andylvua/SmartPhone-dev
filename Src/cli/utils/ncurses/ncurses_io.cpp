//
// Created by Andrew Yaroshevych on 17.01.2023.
//

#include "cli/utils/ncurses/ncurses_io.hpp"
#include "cli/definitions/colors.hpp"
#include <algorithm>

void ColorPrint::initColors() {
    start_color();
    init_pair(WHITE_PAIR, COLOR_WHITE, COLOR_BLACK);
    init_pair(RED_PAIR, COLOR_RED, COLOR_BLACK);
    init_pair(GREEN_PAIR, COLOR_GREEN, COLOR_BLACK);
    init_pair(YELLOW_PAIR, COLOR_YELLOW, COLOR_BLACK);
    init_pair(FILLED_WHITE_PAIR, COLOR_BLACK, COLOR_WHITE);
    init_pair(FILLED_RED_PAIR, COLOR_RED, COLOR_WHITE);
    init_pair(FILLED_GREEN_PAIR, COLOR_GREEN, COLOR_WHITE);
}

void printColored(int color, const QString &text, bool newLine, bool bold) {
    attron(COLOR_PAIR(color));
    if (bold) {
        attron(A_BOLD);
    }

    if (newLine) {
        printw("%s\n", text.toUtf8().data());
        refresh();
    } else {
        printw("%s", text.toUtf8().data());
        refresh();
    }

    attroff(COLOR_PAIR(color));
    attroff(A_BOLD);
}

void printColored(int color, const QString &text, bool newLine, bool bold, WINDOW *window) {
    wattron(window, COLOR_PAIR(color));
    if (bold) {
        wattron(window, A_BOLD);
    }
    if (newLine) {
        wprintw(window, "%s\n", text.toUtf8().data());
    } else {
        wprintw(window, "%s", text.toUtf8().data());
    }
    wrefresh(window);
    wattroff(window, COLOR_PAIR(color));
    wattroff(window, A_BOLD);
}

QString readString(size_t bufferSize) {
    echo();
    char buffer[bufferSize];
    getstr(buffer);
    noecho();
    return {buffer};
}

QString readString(size_t bufferSize, WINDOW *window) {
    char buffer[bufferSize];
    int ch;
    int i = 0;
    while ((ch = wgetch (window)) != '\n') {
        if ((ch == KEY_BACKSPACE || ch == 127) && i > 0) {
            i--;
            wmove(window, getcury (window), getcurx(window)-1);
            wechochar(window, ' ');
            wechochar(window, '\b');
            wrefresh(window);
            continue;
        }

        if (ch == KEY_BACKSPACE || ch == 127) {
            continue;
        }
        buffer[i++] = static_cast<char>(ch);
        wechochar(window, ch);
    }
    wechochar(window, '\n');
    buffer[i] = '\0';
    return {buffer};
}
