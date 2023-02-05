//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#include "cli/utils/ncurses/ncurses_utils.hpp"
#include "cli/utils/ncurses/ncurses_io.hpp"
#include "cli/definitions/colors.hpp"
#include <algorithm>
#include <QTextStream>

std::stringstream NcursesUtils::ncursesBuffer;
std::streambuf *NcursesUtils::oldStreamBuffer;

void NcursesUtils::initScreen() {
    initscr();
    savetty();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    ColorPrint::initColors();
}

void NcursesUtils::releaseScreen() {
    keypad(stdscr, FALSE);
    resetty();
    noraw();
    endwin();
    system("clear");
}

void NcursesUtils::displayPad(const QString &data, QString header) {
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

    long dataRows = std::count(data.begin(), data.end(), '\n') + 1;

    long currentLineLength = 0;
    for (auto c : data) {
        if (c == '\n') {
            if (currentLineLength > screenCols) {
                dataRows += currentLineLength / screenCols;
            }
            currentLineLength = 0;
        } else {
            currentLineLength++;
        }
    }

    long rows = dataRows;
    long cols = screenCols;

    WINDOW *pad = newpad(static_cast<int>(rows), static_cast<int>(cols));

    initscr();
    keypad(pad, TRUE);
    scrollok(pad, TRUE);
    wprintw(pad, "%s", data.toUtf8().data());

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

    delwin(pad);
}

void NcursesUtils::displayPad(QFile &file, QString header) {
    clear();
    move(0, 0);

    header += " (Press 'q' to exit)";
    printColored(FILLED_WHITE_PAIR, header);
    wrefresh(stdscr);

    file.open(QIODevice::Text | QIODevice::ReadOnly);
    QTextStream fileStream(&file);

    int ch;
    int row = 0;
    int col = 0;

    long screenRows;
    long screenCols;
    getmaxyx(stdscr, screenRows, screenCols);
    long dataRows = 1;
    while (!file.atEnd()) {
        QString line = file.readLine();
        if (line.length() > screenCols) {
            dataRows += line.length() / screenCols;
        }
        dataRows++;
    }
    long rows = dataRows;
    long cols = screenCols;
    WINDOW *pad = newpad(static_cast<int>(rows+1), static_cast<int>(cols));


    initscr();
    keypad(pad, TRUE);
    scrollok(pad, TRUE);

    file.seek(0);

    while (!file.atEnd()) {
        QString line = file.readLine();
        wprintw(pad, line.toStdString().c_str());
    }

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
    file.close();

    delwin(pad);
}

void NcursesUtils::clearCurrentLine() {
    move(getcury(stdscr), 0);
    clrtoeol();
}
