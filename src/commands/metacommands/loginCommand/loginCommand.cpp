#include "headers/loginCommand.h"
#include "lockManager/lockManager.h"

LoginCommand::LoginCommand(std::string& currentUser)
    : currentUser(currentUser)
{}

void LoginCommand::execute(const std::vector<std::string>& args) {
    if (args.size() < 2 || args.size () > 2) {
        DISPLAY_MESSAGE("Usage: .login <user> <password>");
        return;
    }

    std::string username = args[0];
    std::string password = args[1];

    INIT_USER_MANAGER;
    if (IS_LOCKED("",LockFileType::USER_INFO)){
        DISPLAY_MESSAGE("user config file is currently locked... try again later");
    }
    LOCK("",LockFileType::USER_INFO,currentUser,"logging in");
    if (userManager.login(username, password)) {
        DISPLAY_MESSAGE("Logged in as: " + username);
        currentUser = username;
        LOG(LogLevel::INFO, "", username, "Logged in.");
    } else {
        DISPLAY_MESSAGE("Invalid username or password.");
        LOG(LogLevel::ERROR, "", username, "Login failed: Invalid username or password.");
    }
    UNLOCK("", LockFileType::USER_INFO);
}
