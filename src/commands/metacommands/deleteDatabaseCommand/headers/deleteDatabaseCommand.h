#ifndef DELETE_DATABASE_COMMAND_H
#define DELETE_DATABASE_COMMAND_H

#include "commands/command.h"
#include "databaseManager/databaseManager.h"
#include <string>
#include "checks.h"
#include <vector>
#include <iostream>
#include "logManager/logManager.h"

class DeleteDatabaseCommand : public Command {
public:
    DeleteDatabaseCommand(std::string& currentUser);
    void execute(const std::vector<std::string>& args) override;

private:
    std::string &currentUser;
};

#endif
