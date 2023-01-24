//
// Created by Andrew Yaroshevych on 17.01.2023.
//

#ifndef PHONE_COLOR_PRINT_H
#define PHONE_COLOR_PRINT_H

#include <iostream>

constexpr const char* RESET = "\033[0m";
constexpr const char* RED = "\033[31m";
constexpr const char* GREEN = "\033[32m";
constexpr const char* YELLOW = "\033[33m";
constexpr const char* WHITE = "\033[37m";
constexpr const char* BOLDRED = "\033[1m\033[31m";
constexpr const char* BOLDGREEN = "\033[1m\033[32m";
constexpr const char* BOLDYELLOW = "\033[1m\033[33m";
constexpr const char* BOLDWHITE = "\033[1m\033[37m";


void printColored(const char *color, const std::string &text, bool newLine = true, std::ostream &outStream = std::cout);

#endif //PHONE_COLOR_PRINT_H
