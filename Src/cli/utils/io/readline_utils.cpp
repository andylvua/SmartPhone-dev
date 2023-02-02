//
// Created by Andrew Yaroshevych on 30.01.2023.
//

#include "cli/utils/io/readline_utils.hpp"
#include <fstream>
#include <vector>
#include <iostream>
#include <readline/readline.h>
#include <algorithm>

const char **words = {nullptr};

char *commandGenerator(const char *text, int state) {
    static size_t listIndex, len;
    const char *name;

    if (!state) {
        listIndex = 0;
        len = strlen(text);
    }

    while ((name = words[listIndex++])) {
        if (strncasecmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    return nullptr;
}

char **commandCompletion(const char *text, [[maybe_unused]] int start, [[maybe_unused]] int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, commandGenerator);
}

void initReadlineCompletions() {
    std::ifstream atCommandsFile("../assets/at_commands.txt");

    if (!atCommandsFile.is_open()) {
        std::cerr << "Could not open completions file" << std::endl;
        words = new const char* {nullptr};
    }

    std::string line;
    std::vector<std::string> atCommands;

    while (std::getline(atCommandsFile, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        atCommands.push_back(line);
    }

    words = new const char*[atCommands.size() + 1];

    for (size_t i = 0; i < atCommands.size(); i++) {
        words[i] = atCommands[i].c_str();
    }

    words[atCommands.size()] = nullptr;
}
