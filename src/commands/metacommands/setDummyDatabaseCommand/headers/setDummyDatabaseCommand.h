#ifndef SET_DUMMY_DATABASE_COMMAND_H
#define SET_DUMMY_DATABASE_COMMAND_H

#include "commands/command.h"
#include <iostream>

class SetDummyDatabaseCommand : public Command {
public:
    SetDummyDatabaseCommand(std::string &currentDatabase) : currentDatabase(currentDatabase) {}
    virtual void execute(const std::vector<std::string>& args) override;

private:
    std::string &currentDatabase;
};

#endif
