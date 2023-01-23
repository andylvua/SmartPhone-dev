//
// Created by Andrew Yaroshevych on 23.01.2023.
//

#ifndef PHONE_OPTION_H
#define PHONE_OPTION_H

#include <QString>
#include <memory>

class Screen;

class Option {
public:
    QString optionName;
    std::function<void()> action;

    Option(QString name, std::function<void()> const& action);

    void execute() const;
};

#endif //PHONE_OPTION_H
