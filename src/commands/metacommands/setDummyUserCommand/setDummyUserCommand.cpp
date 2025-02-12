#include "headers/setDummyUserCommand.h"

void SetDummyUserCommand::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: .set dummy user <username>" << std::endl;
        return;
    }
    currentUser = args[0];
    std::cout << "Dummy user set to: " << currentUser << std::endl;
}
