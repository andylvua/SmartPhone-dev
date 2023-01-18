//
// Created by paul on 1/15/23.
//

#include <utility>

#include "../../Inc/cli/screen.h"

Screen::Screen(QString name, std::shared_ptr<Screen> parentScreen) : parentScreen(std::move(parentScreen)), screenName(std::move(name)) {}

void Screen::addScreenOption(const QString &option) {
    screenOptions.push_back(option);
}

void Screen::removeScreenOption(const QString &option) {
    screenOptions.erase(std::remove(screenOptions.begin(), screenOptions.end(), option), screenOptions.end());
}

void Screen::addNotification(const QString &notification) {
    notifications.push_back(notification);
}

void Screen::removeNotification(const QString &notification) {
    notifications.erase(std::remove(notifications.begin(), notifications.end(), notification), notifications.end());
}

void Screen::removeScreenOption(int index) {
    screenOptions.erase(screenOptions.begin() + index);
}
