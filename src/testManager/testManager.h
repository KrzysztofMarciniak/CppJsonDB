#ifndef TEST_MANAGER_H
#define TEST_MANAGER_H

#include <string>
#include <vector>

class TestManager {
public:
    explicit TestManager(const std::string& configFilePath);

    std::vector<std::string> loadFile();
    void assignToArgs(std::vector<std::string>& args);

private:
    std::string configFilePath;
    std::vector<std::string> testArgs;
};

#endif
