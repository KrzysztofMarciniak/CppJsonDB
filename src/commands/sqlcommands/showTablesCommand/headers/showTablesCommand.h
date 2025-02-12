#ifndef SHOWTABLES_COMMAND_H
#define SHOWTABLES_COMMAND_H

#include "commands/command.h"
#include <iostream>
#include "sqlManager/sqlManager.h"
#include "logManager/logManager.h"
#include "databaseManager/databaseManager.h"
#include "lockManager/lockManager.h"
#include "checks.h"
class ShowTablesCommand : public Command {
public:
    ShowTablesCommand(std::string& currentUser, std::string& currentDatabase);
    virtual void execute(const std::vector<std::string>& args) override;
private:
    std::string& currentUser;
    std::string& currentDatabase;
};

#endif
