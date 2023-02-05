//
// Created by Andrew Yaroshevych on 17.01.2023.
//

#ifndef PHONE_NCURSES_IO_HPP
#define PHONE_NCURSES_IO_HPP

#include <iostream>
#include <ncurses.h>
#include <QString>

class ColorPrint {
public:
    static void initColors();
};

void printColored(int color, const QString &text, bool newLine = true, bool bold = false);

void printColored(int color, const QString &text, bool newLine, bool bold, WINDOW *window);

QString readString(size_t bufferSize = 256);

QString readString(size_t bufferSize, WINDOW *window);

#endif //PHONE_NCURSES_IO_HPP
