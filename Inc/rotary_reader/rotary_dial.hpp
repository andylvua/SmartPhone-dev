//
// Created by Yaroslav Korch on 17.01.2023.
//

#ifndef PHONE_ROTARY_DIAL_HPP
#define PHONE_ROTARY_DIAL_HPP
#include <iostream>
#include "../cli/ncurses_io.hpp"

#ifdef BUILD_ON_RASPBERRY
class RotaryDial {
    const int pinDial = 4;  // blue
    const int pinPulse = 5;  // yellow
public:
    char phone_number[12];

    void setup();
    char * listen_for_number(std::ostream &outStream = std::cout);
};
#endif //BUILD_ON_RASPBERRY

#endif //PHONE_ROTARY_DIAL_HPP
