//
// Created by Andrew Yaroshevych on 17.01.2023.
//

#include "../../Inc/cli/color_print.h"

void ColorPrint::initColors() {
    start_color();
    init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(RED, COLOR_RED, COLOR_BLACK);
    init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(FILLED_WHITE, COLOR_BLACK, COLOR_WHITE);
}

void printColored(int color, const std::string &text, bool newLine, bool bold) {
    attron(COLOR_PAIR(color));
    if (bold) {
        attron(A_BOLD);
    }

    if (newLine) {
        printw("%s\n", text.c_str());
    } else {
        printw("%s", text.c_str());
        refresh();
    }

    attroff(COLOR_PAIR(color));
    attroff(A_BOLD);
}
