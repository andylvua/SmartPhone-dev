//
// Created by paul on 1/15/23.
//

#ifndef PHONE_SCREEN_HPP
#define PHONE_SCREEN_HPP

#include <vector>
#include <QString>
#include <memory>
#include <unordered_map>
#include <QSharedPointer>
#include "option.hpp"
#include "modem/media_types.hpp"

class CLI;

using OptionsMap = QHash<QString, QSharedPointer<Option>>;

class Screen {
public:
    QString screenName;
    QVector<QSharedPointer<Screen>> childScreens;
    QSharedPointer<Screen> parentScreen;

    QVector<QSharedPointer<Option>> screenOptions;
    OptionsMap optionsMap;

    int activeOption = -1;

    QVector<QString> notifications = {};

    Screen(QString name, QSharedPointer<Screen> parentScreen);

    void render();

    void addScreenOption(const QString &name, std::function<void()> const &action);

    void addScreenOption(const QString &name, std::function<void()> const &action, bool switcher);

    void addNotification(const QString &notification);

    [[nodiscard]] int getPagesCount() const;

    [[nodiscard]] int getActivePage() const;

    [[nodiscard]] int getActiveOption() const;

    [[nodiscard]] int getMaxOptionsPerPage() const;

    [[nodiscard]] bool isFirstPage() const;

    [[nodiscard]] bool isLastPage() const;

};

class ContactScreen : public Screen {
public:
    Contact contact;

    ContactScreen(QSharedPointer<Screen> parentScreen, const Contact &contact, CLI &cli);
};

#endif //PHONE_SCREEN_HPP
