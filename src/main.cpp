#include "commandFactory/headers/commandFactory.h"
#include "repl/headers/repl.h"
#include "testManager/testManager.h"
#include <vector>
#include <string>

const bool TEST_MODE = true;
const std::string TEST_CONFIG_PATH = "tests.cfg";
const bool SAVE_EXECUTION_DETAILS = true;

int main(int argc, char* argv[]) {
    std::string currentUser, currentDatabase;
    CommandFactory factory(currentUser, currentDatabase);
    Repl repl(factory, currentUser, currentDatabase);
    repl.setSaveExecutionDetails(SAVE_EXECUTION_DETAILS);
    std::vector<std::string> args;
    if (TEST_MODE) {
        TestManager testManager(TEST_CONFIG_PATH);
        args = testManager.loadFile();
    } else if (argc > 1) {
        args.assign(argv + 1, argv + argc);
    }
    if (!args.empty()) {
        repl.processStartupCommands(args);
    } else {
        repl.start();
    }
    return 0;
}
