#ifndef TEST_INJECTOR_H
#define TEST_INJECTOR_H

#include <string>
#include <vector>

namespace TestInjector {
    extern const std::string configFilePath;
    std::vector<std::string> loadTestCommands();
}

#endif
