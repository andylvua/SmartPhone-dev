# SmartPhone
## This project is a simple implementation of a phone based on the GSM/GPRS module A6. It is based on the RaspberryPI3 platform. The project is based on the following components:
* Raspberry PI3 B+
* A6/A9 GSM/GPRS module
* 2.8" TFT LCD screen
* Microphone
* Rotary dial
## The project is divided into 2 parts:
* The backend part: This part is responsible for the communication with the GSM/GPRS module. It is based on the AT commands sent via SerialPort under Qt framework.
* The frontend part: This part is responsible for the communication with the user. It is based on the LCD screen and the microphone. It is based on the Qt and ncurses.
## Prerequisites
* Qt 5.15.2
* ncurses 6.2
* WiringPi(optional for the rotary dial)
* readline 
## Installation
Assuming that you have installed needed packages, you can build the project by running the following commands(can be installed on Raspberry PI3 or on a PC):
```
$ git clone
$ cd SmartPhone
$ cmake .
```
## Usage
1. Make sure that the A6/A9 module is connected to the Raspberry PI3.
2. Run the following command:
```
$ ./SmartPhone
```
It's recommended to run the program as root.
```
$ sudo ./SmartPhone
```
You can navigate through the menu by using the keyboard arrows.