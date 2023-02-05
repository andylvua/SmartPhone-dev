//
// Created by Andrew Yaroshevych on 05.02.2023.
//

#ifndef PHONE_ASSERTIONS_HPP
#define PHONE_ASSERTIONS_HPP

#include "cli/definitions/colors.hpp"
#include "cli/utils/ncurses/ncurses_io.hpp"

[[nodiscard]] bool assertNumberCorrectness(QString &number);

#endif //PHONE_ASSERTIONS_HPP
