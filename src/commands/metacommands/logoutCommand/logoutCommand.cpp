#include "headers/logoutCommand.h"

LogoutCommand::LogoutCommand(std::string& currentUser, std::string& currentDatabase)
    : currentUser(currentUser), currentDatabase(currentDatabase)
{}

void LogoutCommand::execute(const std::vector<std::string>& args) {
    currentUser.clear();
    currentDatabase.clear();
}