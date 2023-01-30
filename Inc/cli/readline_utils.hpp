//
// Created by Andrew Yaroshevych on 30.01.2023.
//

#ifndef PHONE_READLINE_UTILS_HPP
#define PHONE_READLINE_UTILS_HPP

char *commandGenerator(const char *text, int state);
char **commandCompletion(const char *text, int start, int end);
void initReadlineCompletions();

#endif //PHONE_READLINE_UTILS_HPP
