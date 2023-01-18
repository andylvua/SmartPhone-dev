//
// Created by Yaroslav Korch on 17.01.2023.
//

#include "rotary_dial.h"

#include <iostream>		// Include all needed libraries here
#include <wiringPi.h>




void RotaryDial::setup(){
    wiringPiSetup();
    pinMode(pinDial, INPUT);
    pinMode(pinPulse, INPUT);
    pullUpDnControl(pinDial, PUD_UP);
    pullUpDnControl(pinPulse, PUD_UP);
}

char * RotaryDial::listen_for_number(){
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
                //~ std::cout << count << std::endl;
            }
        }
    }
    return phone_number;
}

void RotaryDial::print_number(){
    for (int i = 0; i < 12; i++){
        std::cout << phone_number[i];
    }
}


//~ int main(){
//~ RotaryDial rtx;
//~ rtx.setup();
//~ char * phone = rtx.listen_for_number();
//~ rtx.print_number();
//~ return 0;
//~ }