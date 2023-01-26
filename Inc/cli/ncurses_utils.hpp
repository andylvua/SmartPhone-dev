//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#ifndef PHONE_NCURSES_UTILS_HPP
#define PHONE_NCURSES_UTILS_HPP

#include <ncurses.h>
#include <string>

void initScreen();

void releaseScreen();

void displayPad(const std::string &data, std::string header);

WINDOW *createConsole(std::string header);

#endif //PHONE_NCURSES_UTILS_HPP
