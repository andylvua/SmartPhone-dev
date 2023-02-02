//
// Created by Yaroslav Korch on 17.01.2023.
//

#ifndef PHONE_ROTARY_DIAL_HPP
#define PHONE_ROTARY_DIAL_HPP
#include <iostream>
#include "cli/utils/io/ncurses_io.hpp"

#ifdef BUILD_ON_RASPBERRY
#define pinDial 4
#define pinPulse 5

class RotaryDial {
public:
    char phone_number[12];

    static void setup();
    char * listen_for_number();
};
#endif //BUILD_ON_RASPBERRY

#endif //PHONE_ROTARY_DIAL_HPP
