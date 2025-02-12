#ifndef LOGOUT_COMMAND_H
#define LOGOUT_COMMAND_H

#include "commands/command.h"
#include <unordered_map>
#include <string>
#include <iostream>
#include "userManager/userManager.h"

class LogoutCommand : public Command {
public:
    LogoutCommand(std::string& currentUser, std::string& currentDatabase); 
    virtual void execute(const std::vector<std::string>& args) override;
private:
    std::string& currentUser;
    std::string& currentDatabase;
};

#endif
