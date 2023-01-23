//
// Created by paul on 1/15/23.
//

#include <utility>
#include <ncurses.h>
#include "../../Inc/cli/screen.h"
#include "../../Inc/cli/color_print.h"

Screen::Screen(QString name, std::shared_ptr<Screen> parentScreen) : screenName(std::move(name)),
                                                                     parentScreen(std::move(parentScreen)) {}

void Screen::initScreen() {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    ColorPrint::initColors();
}

void Screen::releaseScreen() {
    refresh();
    getch();
    endwin();
}

void Screen::addScreenOption(const QString &name, std::function<void()> const& action) {
    Option option(name, action);
    screenOptions.push_back(option);
}

int Screen::getActiveOption() const {
    if (activeOption == -1) {
        return -1;
    }

    auto optionsSize = static_cast<int>(screenOptions.size());

    return activeOption % optionsSize;
}

void Screen::removeScreenOption(const QString &option) {
    screenOptions.erase(std::remove_if(screenOptions.begin(), screenOptions.end(),
                                       [&option](const Option &screenOption) {
                                           return screenOption.optionName == option;
                                       }), screenOptions.end());
}

void Screen::addNotification(const QString &notification) {
    notifications.push_back(notification);
}

void Screen::removeNotification(const QString &notification) {
    notifications.erase(std::remove(notifications.begin(), notifications.end(), notification), notifications.end());
}

void Screen::removeScreenOption(int index) {
    screenOptions.erase(screenOptions.begin() + index);
}

void Screen::addChildScreen(const std::shared_ptr<Screen> &screen) {
    childScreens.push_back(screen);
}
