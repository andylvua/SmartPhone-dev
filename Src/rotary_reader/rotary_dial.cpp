//
// Created by Yaroslav Korch on 17.01.2023.
//

#ifdef BUILD_ON_RASPBERRY
#include <wiringPi.h>
#include "rotary_reader/rotary_dial.hpp"
#include "cli/defenitions/colors.hpp"

void RotaryDial::setup(){
    wiringPiSetup();
    pinMode(pinDial, INPUT);
    pinMode(pinPulse, INPUT);
    pullUpDnControl(pinDial, PUD_UP);
    pullUpDnControl(pinPulse, PUD_UP);
}

char * RotaryDial::listen_for_number() {
    int numbers_typed = 0;
    int count = 0;
    while (numbers_typed < 12) {
        if(!digitalRead(pinDial)){
            delay(20);
            count = 0;
            while(!digitalRead(pinDial)){
                if(digitalRead(pinPulse)){
                    delay(5);
                    while(digitalRead(pinPulse)){delay(5);}
                    delay(5);
                    count++;
                }
            }
            delay(20);
            if(count){
                if(count >= 10){count=0;}

                phone_number[numbers_typed++] = '0' + count;
                printColored(WHITE_PAIR, std::to_string(count), false);
            }
        }
    }
    std::cout<<std::endl;
    return phone_number;
}

#endif //BUILD_ON_RASPBERRY
