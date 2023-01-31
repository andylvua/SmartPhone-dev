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

    Option(QString name, std::function<void()> const& action);

    Option(QString name, std::function<void()> const& action, bool isSwitcher, bool switcher);

    void execute() const;

    void switchState();

    bool getState();
};
//
//class OptionSwitcher: public Option {
//public:
//    OptionSwitcher(QString name, std::function<void()> const& action, bool switcher, bool isSwitcher = true);
//    bool switcher;
//
//    void switchState();
//
//    bool getState();
//};

#endif //PHONE_OPTION_HPP
