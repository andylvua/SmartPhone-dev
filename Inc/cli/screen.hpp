//
// Created by paul on 1/15/23.
//

#ifndef PHONE_SCREEN_HPP
#define PHONE_SCREEN_HPP

#include <vector>
#include <QString>
#include <memory>
#include "option.hpp"

class Screen {
public:
    QString screenName;
    std::vector<std::shared_ptr<Screen>> childScreens;
    std::shared_ptr<Screen> parentScreen;
    std::vector<Option> screenOptions;
    int activeOption = -1;
    std::vector<QString> notifications = {};

    Screen(QString name, std::shared_ptr<Screen> parentScreen);

    void addScreenOption(const QString &name, std::function<void()> const& action);

    [[nodiscard]] int getActiveOption() const;

    void addNotification(const QString &notification);

    void removeScreenOption(const QString &option);

    void removeScreenOption(int index);

    void removeNotification(const QString &notification);
};

#endif //PHONE_SCREEN_HPP