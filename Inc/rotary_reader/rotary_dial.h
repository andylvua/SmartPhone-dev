//
// Created by Yaroslav Korch on 17.01.2023.
//

#ifndef LEARNGIT_ROTARY_DIAL_H
#define LEARNGIT_ROTARY_DIAL_H


class RotaryDial {
    const int pinDial = 4;  // blue
    const int pinPulse = 5;  // yellow
public:
    char phone_number[12];

    void setup();
    char * listen_for_number();
    void print_number();
};


#endif //LEARNGIT_ROTARY_DIAL_H