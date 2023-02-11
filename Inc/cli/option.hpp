//
// Created by Andrew Yaroshevych on 23.01.2023.
//

#ifndef PHONE_OPTION_HPP
#define PHONE_OPTION_HPP

#include <QString>
#include <memory>
#include <functional>

class Screen;

class Option {
public:
    QString optionName;
    std::function<void()> action;
    const bool isSwitcher = false;
    bool switcher = false;
    bool isAvailable = true;

    Option(QString name, std::function<void()> const &action);

    Option(QString name, std::function<void()> const &action, bool switcher);

    void execute() const;

    void switchState();

    [[nodiscard]] bool getState() const;
};

#endif //PHONE_OPTION_HPP
