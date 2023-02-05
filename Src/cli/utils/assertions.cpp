//
// Created by Andrew Yaroshevych on 05.02.2023.
//

#include "cli/utils/assertions.hpp"

bool assertNumberCorrectness(QString &number) {
    if (number[0] != '+') {
        number = '+' + number;
    }

    if (number.length() != 13) {
        printColored(RED_PAIR, "Invalid number: " + number);
        return false;
    }

    for (int i = 1; i < number.length(); ++i) {
        if (!number[i].isDigit()) {
            printColored(RED_PAIR, "Invalid number");
            return false;
        }
    }

    return true;
}
