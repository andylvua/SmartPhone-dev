//
// Created by Andrew Yaroshevych on 17.01.2023.
//

#ifndef PHONE_COLOR_PRINT_H
#define PHONE_COLOR_PRINT_H

#include <iostream>

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


void printColored(const char* color, std::string text, bool newLine = true, std::ostream &outStream = std::cout);

#endif //PHONE_COLOR_PRINT_H
