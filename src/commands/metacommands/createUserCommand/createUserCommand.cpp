#include "headers/createUserCommand.h"
#include "lockManager/lockManager.h"
CreateUserCommand::CreateUserCommand() {}

void CreateUserCommand::execute(const std::vector<std::string>& args) {
    if (args.size() < 2 || args.size() > 2) {
        DISPLAY_MESSAGE("Usage: .create user <username> <password>");
        return;
    }
    LockFileType LockFileTypeToUse = LockFileType::USER_INFO;




    if (IS_LOCKED("", LockFileTypeToUse)){
        DISPLAY_MESSAGE("users config file is currently locked... try again later");
        return;
    }
    LOCK("", LockFileTypeToUse, "", "CREATE USER");

    INIT_USER_MANAGER;
    std::string username = args[0];
    std::string password = args[1];

    if (userManager.createUser(username, password)) {
        DISPLAY_MESSAGE("User created: " + username);
        LOG(LogLevel::INFO, "", "", "User created: " + username);
    } else {
        DISPLAY_MESSAGE("Failed to create user: " + username);
        LOG(LogLevel::ERROR, "", "", "Failed to create user: " + username);
    }
    UNLOCK("", LockFileTypeToUse);
}
