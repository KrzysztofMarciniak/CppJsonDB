#include "headers/exitCommand.h"

void ExitCommand::execute(const std::vector<std::string>& args) {
    std::cout << "Exiting..." << std::endl;
    std::exit(0);
}
