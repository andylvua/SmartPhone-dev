//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#ifndef PHONE_NCURSES_UTILS_HPP
#define PHONE_NCURSES_UTILS_HPP

#include <ncurses.h>
#include <string>
#include <sstream>
#include <QFile>

class NcursesUtils {
public:
    [[maybe_unused]] static std::stringstream ncursesBuffer;
    [[maybe_unused]] static std::streambuf *oldStreamBuffer;

    static void initScreen();

    static void releaseScreen();

    static void displayPad(const QString &data, QString header);

    static void displayPad(QFile &file, QString header);
};

#endif //PHONE_NCURSES_UTILS_HPP
