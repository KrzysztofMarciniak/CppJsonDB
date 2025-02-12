#ifndef CREATE_USER_COMMAND_H
#define CREATE_USER_COMMAND_H

#include "commands/command.h"
#include "userManager/userManager.h"
#include <iostream>
#include <vector>
#include "logManager/logManager.h" 

class CreateUserCommand : public Command {
public:
    CreateUserCommand();

    void execute(const std::vector<std::string>& args) override;


};

#endif
