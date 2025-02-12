#include "testManager.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <utility>

TestManager::TestManager(const std::string& configFilePath)
    : configFilePath(configFilePath) {}

std::vector<std::string> TestManager::loadFile() {
    std::vector<std::string> args;
    std::ifstream file(configFilePath);
    if (!file) {
        std::cerr << "Error: Could not open file " << configFilePath << std::endl;
        return args;
    }

    std::string allCommands;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line[0] != '#' && line[0] != '[') {
            if (!allCommands.empty()) {
                allCommands += "//";
            }
            allCommands += line;
        }
    }
    args.push_back(allCommands);

    std::cout << "Loaded commands from " << configFilePath << std::endl;
    return args;
}

void TestManager::assignToArgs(std::vector<std::string>& args) {
    testArgs = std::move(args);
}
