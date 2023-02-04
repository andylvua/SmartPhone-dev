//
// Created by Andrew Yaroshevych on 26.01.2023.
//

#ifndef PHONE_CLI_MACROS_HPP
#define PHONE_CLI_MACROS_HPP

#define SCREEN_SHARED_PTR(name, parentScreen) QSharedPointer<Screen>::create(name, parentScreen)

#define GO_BACK [&](){gotoParentScreen();}
#define CHANGE_SCREEN(screenName) [&](){changeScreen(screenName);}
#define EXECUTE_METHOD(method) [&](){method();}

#endif //PHONE_CLI_MACROS_HPP
