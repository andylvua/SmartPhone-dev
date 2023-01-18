//
// Created by Andrew Yaroshevych on 17.01.2023.
//

#include "../../Inc/cli/color_print.h"

void printColored(const char* color, const std::string &text, bool newLine, std::ostream &outStream) {
    if (newLine) {
        outStream << color << text << RESET << std::endl;
    } else {
        outStream << color << text << RESET;
        outStream.flush();
    }
}
