//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#include "../../Inc/cli/ncurses_utils.hpp"
#include "../../Inc/cli/ncurses_io.hpp"
#include "../../Inc/cli/colors.hpp"
#include <algorithm>
#include <termios.h>

const auto terminalState = termios{};

void initScreen() {
    tcgetattr(0, const_cast<termios *>(&terminalState));
    initscr();
    savetty();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    ColorPrint::initColors();
}

void releaseScreen() {
    refresh();
    keypad(stdscr, FALSE);
    resetty();
    noraw();
    endwin();
    tcsetattr(0, TCSANOW, &terminalState);
}

void displayPad(const std::string &data, std::string header) {
    clear();
    move(0, 0);

    header += " (Press 'q' to exit)";
    printColored(FILLED_WHITE_PAIR, header);
    wrefresh(stdscr);

    int ch;
    int row = 0;
    int col = 0;

    long screenRows;
    long screenCols;
    getmaxyx(stdscr, screenRows, screenCols);

    long rows = std::count(data.begin(), data.end(), '\n') + 1;
    long cols = screenCols;

    WINDOW *pad = newpad(static_cast<int>(rows), static_cast<int>(cols));

    initscr();
    keypad(pad, TRUE);
    scrollok(pad, TRUE);
    wprintw(pad, data.c_str());

    prefresh(pad, row, col, 1, 0, static_cast<int>(screenRows - 1), static_cast<int>(screenCols - 1));

    while ((ch = wgetch(pad)) != 'q') {
        switch (ch) {
            case KEY_UP:
                if (row > 0) {
                    row--;
                }
                break;
            case KEY_DOWN:
                if (row < rows - screenRows) {
                    row++;
                }
                break;
            case KEY_LEFT:
                if (col > 0) {
                    col--;
                }
                break;
            case KEY_RIGHT:
                if (col < cols - screenCols) {
                    col++;
                }
                break;

            default: break;
        }
        prefresh(pad, row, col, 1, 0, static_cast<int>(screenRows - 1), static_cast<int>(screenCols - 1));
    }
}

WINDOW *createConsole(std::string header) {
    header += ". Type 'exit' to exit";

    clear();

    printColored(FILLED_WHITE_PAIR, header);
    wrefresh(stdscr);

    long screenRows;
    long screenCols;
    getmaxyx(stdscr, screenRows, screenCols);

    WINDOW *console = newwin(static_cast<int>(screenRows - 1), static_cast<int>(screenCols), 1, 0);

    initscr();
    keypad(console, TRUE);
    scrollok(console, TRUE);
    curs_set(1);

    return console;
}
