//
// Created by Andrew Yaroshevych on 23.01.2023.
//


#include <utility>

#include "cli/option.hpp"
#include "cli/defenitions/colors.hpp"
#include "cli/utils/io/ncurses_io.hpp"
#include <QThread>


Option::Option(QString name, std::function<void()> const& action) : optionName(std::move(name)), action(action) {};

Option::Option(QString name, std::function<void()> const& action, bool isSwitcher, bool switcher) : optionName(std::move(name)), action(action), isSwitcher(isSwitcher), switcher(switcher) {};

void Option::execute() const {
    if (isAvailable){
        action();
    } else {
        printColored(RED_PAIR, "This option is not available", false);
        QThread::msleep(1000);
        deleteln();
        move(getcury(stdscr), 0);
    }
}

void Option::switchState() {
    switcher = !switcher;
}

bool Option::getState() const {
    return switcher;
}
