#ifndef DROPTABLE_COMMAND_H
#define DROPTABLE_COMMAND_H

#include "commands/command.h"
#include <iostream>
#include "sqlManager/sqlManager.h"
#include "logManager/logManager.h"
#include "databaseManager/databaseManager.h"
#include <vector>
#include <string>
#include "lockManager/lockManager.h"
#include <algorithm>
#include "checks.h"
class DropTableCommand : public Command {
public:
    DropTableCommand(std::string& currentUser, std::string& currentDatabase);
    virtual void execute(const std::vector<std::string>& args) override;
private:
    std::string& currentUser;
    std::string& currentDatabase;
};

#endif
