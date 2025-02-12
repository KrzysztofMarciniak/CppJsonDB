#include "testInjector.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace TestInjector {
    const std::string configFilePath = "tests.cfg";


    std::vector<std::string> loadTestCommands() {
        std::vector<std::string> args;
        std::ifstream file(configFilePath);

        if (!file) {
            std::cerr << "Error: Could not open file " << configFilePath << std::endl;
            return args;
        }

        std::string line;
        std::string allCommands;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                if (!allCommands.empty()) {
                    allCommands += "//";
                }
                allCommands += line;
            }
        }

        args.push_back(allCommands);
        return args;
    }
}
