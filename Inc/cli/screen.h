//
// Created by paul on 1/15/23.
//

#ifndef PHONE_SCREEN_H
#define PHONE_SCREEN_H

#include <vector>
#include <QString>

class Screen {
public:
    std::vector<Screen> childScreens;
    Screen* parentScreen;
    QString screenName;
    std::vector<QString> screenOptions;

    Screen(QString name, Screen* parentScreen);

    void addChildScreen(Screen &screen);

    void addParentScreen(Screen* screen);

    void addScreenOption(QString option);

    void removeScreenOption(QString option);

    void removeScreenOption(int index);

};
#endif //PHONE_SCREEN_H
