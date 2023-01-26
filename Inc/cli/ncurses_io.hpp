//
// Created by Andrew Yaroshevych on 17.01.2023.
//

#ifndef PHONE_NCURSES_IO_HPP
#define PHONE_NCURSES_IO_HPP

#include <iostream>
#include <ncurses.h>

class ColorPrint {
public:
    static void initColors();
};

void printColored(int color, const std::string &text, bool newLine = true, bool bold = false);

void printColored(int color, const std::string &text, bool newLine, bool bold, WINDOW *window);

std::string readString(size_t bufferSize = 256);

std::string readString(size_t bufferSize, WINDOW *window);

#endif //PHONE_NCURSES_IO_HPP
