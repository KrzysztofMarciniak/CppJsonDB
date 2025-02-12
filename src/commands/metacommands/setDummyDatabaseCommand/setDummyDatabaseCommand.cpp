#include "headers/setDummyDatabaseCommand.h"

void SetDummyDatabaseCommand::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: .set dummy database <database_name>" << std::endl;
        return;
    }
    currentDatabase = args[0];
    std::cout << "Dummy database set to: " << currentDatabase << std::endl;
}
