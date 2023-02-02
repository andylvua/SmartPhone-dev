//
// Created by paul on 1/15/23.
//

#ifndef PHONE_SCREEN_HPP
#define PHONE_SCREEN_HPP

#include <vector>
#include <QString>
#include <memory>
#include "option.hpp"
#include "modem/media_types.hpp"

class CLI;

class Screen {
public:
    QString screenName;
    std::vector<std::shared_ptr<Screen>> childScreens;
    std::shared_ptr<Screen> parentScreen;
    std::vector<std::shared_ptr<Option>> screenOptions;
    int activeOption = -1;

    std::vector<QString> notifications = {};

    Screen(QString name, std::shared_ptr<Screen> parentScreen);

    void addScreenOption(const QString &name, std::function<void()> const& action);

    void addScreenOption(const QString &name, std::function<void()> const& action, bool switcher);

    void removeScreenOption(int index);

    [[nodiscard]] int getActiveOption() const;

    [[nodiscard]] int getMaxOptionsPerPage() const;

    [[nodiscard]] bool isFirstPage() const;

    [[nodiscard]] bool isLastPage() const;

    void addNotification(const QString &notification);

};

class ContactScreen : public Screen {
public:
    Contact contact;

    ContactScreen(std::shared_ptr<Screen> parentScreen, const Contact& contact, CLI &cli);
};

#endif //PHONE_SCREEN_HPP
