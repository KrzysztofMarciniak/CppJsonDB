#ifndef LOGIN_COMMAND_H
#define LOGIN_COMMAND_H

#include "commands/command.h"
#include <unordered_map>
#include <string>
#include <iostream>
#include "userManager/userManager.h"
#include "logManager/logManager.h"
class LoginCommand : public Command {
public:
    LoginCommand(std::string& currentUser);
    virtual void execute(const std::vector<std::string>& args) override;

private:
    std::string& currentUser;
};

#endif
