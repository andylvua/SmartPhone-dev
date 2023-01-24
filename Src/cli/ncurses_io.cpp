//
// Created by Andrew Yaroshevych on 17.01.2023.
//

#include "../../Inc/cli/ncurses_io.h"

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

std::string readString(size_t bufferSize) {
    echo();
    char buffer[bufferSize];
    getstr(buffer);
    noecho();
    return {buffer};
}

void displayPad(const std::string &data, std::string header) {
    clear();
    move(0, 0);

    header += " (Press 'q' to exit)";
    printColored(FILLED_WHITE, header);
    wrefresh(stdscr);

    int ch;
    int row = 0;
    int col = 0;

    long screen_rows;
    long screen_cols;
    getmaxyx(stdscr, screen_rows, screen_cols);

    long rows = std::count(data.begin(), data.end(), '\n') + 1;
    long cols = screen_cols;

    WINDOW *pad = newpad(static_cast<int>(rows), static_cast<int>(cols));

    initscr();
    keypad(pad, TRUE);
    scrollok(pad, TRUE);
    wprintw(pad, data.c_str());

    prefresh(pad, row, col, 1, 0, static_cast<int>(screen_rows - 1), static_cast<int>(screen_cols - 1));

    while ((ch = wgetch(pad)) != 'q') {
        switch (ch) {
            case KEY_UP:
                if (row > 0) {
                    row--;
                }
                break;
            case KEY_DOWN:
                if (row < rows - screen_rows) {
                    row++;
                }
                break;
            case KEY_LEFT:
                if (col > 0) {
                    col--;
                }
                break;
            case KEY_RIGHT:
                if (col < cols - screen_cols) {
                    col++;
                }
                break;

            default: break;
        }
        prefresh(pad, row, col, 1, 0, static_cast<int>(screen_rows - 1), static_cast<int>(screen_cols - 1));
    }
}
