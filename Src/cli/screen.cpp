//
// Created by paul on 1/15/23.
//

#include "../../Inc/cli/screen.h"

Screen::Screen(QString name, Screen* parentScreen) : screenName(name), parentScreen(parentScreen) {}

void Screen::addChildScreen(Screen &screen) {
    childScreens.push_back(screen);
}

void Screen::addParentScreen(Screen *screen) {
    parentScreen = screen;
}

void Screen::addScreenOption(QString option) {
    screenOptions.push_back(option);
}

void Screen::removeScreenOption(QString option) {
    screenOptions.erase(std::remove(screenOptions.begin(), screenOptions.end(), option), screenOptions.end());
}
void Screen::addNotification(QString notification) {
    notifications.push_back(notification);
}

void Screen::removeNotification(QString notification) {
    notifications.erase(std::remove(notifications.begin(), notifications.end(), notification), notifications.end());
}

void Screen::removeScreenOption(int index) {
    screenOptions.erase(screenOptions.begin() + index);
}


