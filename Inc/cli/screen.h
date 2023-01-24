//
// Created by paul on 1/15/23.
//

#ifndef PHONE_SCREEN_H
#define PHONE_SCREEN_H

#include <vector>
#include <QString>
#include <memory>

class Screen {
public:
    std::vector<Screen> childScreens;
    std::shared_ptr<Screen> parentScreen;
    QString screenName;
    std::vector<QString> screenOptions;
    std::vector<QString> notifications = {};

    Screen(QString name, std::shared_ptr<Screen> parentScreen);

    void addScreenOption(const QString &option);

    void removeScreenOption(const QString &option);

    void removeScreenOption(int index);

    void addNotification(const QString &notification);

    void removeNotification(const QString &notification);

};

#endif //PHONE_SCREEN_H
