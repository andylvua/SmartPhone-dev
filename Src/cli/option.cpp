//
// Created by Andrew Yaroshevych on 23.01.2023.
//


#include <utility>

#include "../../Inc/cli/option.hpp"


Option::Option(QString name, std::function<void()> const& action) : optionName(std::move(name)), action(action) {};

Option::Option(QString name, std::function<void()> const& action, bool isSwitcher, bool switcher) : optionName(std::move(name)), action(action), isSwitcher(isSwitcher), switcher(switcher) {};

void Option::execute() const {
    action();
}

void Option::switchState() {
    switcher = !switcher;
}
bool Option::getState() {
    return switcher;
}


