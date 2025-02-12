#ifndef EXIT_COMMAND_H
#define EXIT_COMMAND_H

#include "commands/command.h"
#include <iostream>
#include <cstdlib>

class ExitCommand : public Command {
public:
    virtual void execute(const std::vector<std::string>& args) override;
};

#endif
