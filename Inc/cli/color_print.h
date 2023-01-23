//
// Created by Andrew Yaroshevych on 17.01.2023.
//

#ifndef PHONE_COLOR_PRINT_H
#define PHONE_COLOR_PRINT_H

#include <iostream>
#include <ncurses.h>

constexpr int WHITE = 1;
constexpr int RED = 2;
constexpr int GREEN = 3;
constexpr int YELLOW = 4;
constexpr int FILLED_WHITE = 5;

class ColorPrint {
public:
    static void initColors();

    static void printColored(const char *color, const std::string &text, bool newLine = true, std::ostream &outStream = std::cout);

    static void printwColored(const char *color, const std::string &text, bool newLine = true);
};

void printColored(int color, const std::string &text, bool newLine = true, bool bold = false);

#endif //PHONE_COLOR_PRINT_H
