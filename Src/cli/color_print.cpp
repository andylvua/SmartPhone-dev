//
// Created by Andrew Yaroshevych on 17.01.2023.
//

#include "../../Inc/cli/color_print.h"

void printColored(const char* color, std::string text, std::ostream &outStream) {
    outStream << color << text << RESET << std::endl;
}
