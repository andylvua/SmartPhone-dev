//
// Created by Andrew Yaroshevych on 23.01.2023.
//


#include <utility>

#include "../../Inc/cli/option.hpp"


Option::Option(QString name, std::function<void()> const& action) : optionName(std::move(name)), action(action) {}

void Option::execute() const {
    action();
}
