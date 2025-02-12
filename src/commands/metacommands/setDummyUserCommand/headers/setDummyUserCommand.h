#ifndef SET_DUMMY_USER_COMMAND_H
#define SET_DUMMY_USER_COMMAND_H

#include "commands/command.h"
#include <iostream>

class SetDummyUserCommand : public Command {
public:
    SetDummyUserCommand(std::string &currentUser) : currentUser(currentUser) {}
    virtual void execute(const std::vector<std::string>& args) override;

private:
    std::string &currentUser;
};

#endif
