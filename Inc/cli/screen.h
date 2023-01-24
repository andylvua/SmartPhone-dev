//
// Created by paul on 1/15/23.
//

#ifndef PHONE_SCREEN_H
#define PHONE_SCREEN_H

#include <vector>
#include <QString>
#include <memory>
#include "option.h"

class Screen {
public:
    QString screenName;
    std::vector<std::shared_ptr<Screen>> childScreens;
    std::shared_ptr<Screen> parentScreen;
    std::vector<Option> screenOptions;
    int activeOption = -1;
    std::vector<QString> notifications = {};

    Screen(QString name, std::shared_ptr<Screen> parentScreen);

    static void initScreen();

    static void releaseScreen();

    void addScreenOption(const QString &name, std::function<void()> const& action);

    int getActiveOption() const;

    void removeScreenOption(const QString &option);

    void removeScreenOption(int index);

    void addNotification(const QString &notification);

    void removeNotification(const QString &notification);

    void addChildScreen(const std::shared_ptr<Screen> &screen);
};

#endif //PHONE_SCREEN_H
