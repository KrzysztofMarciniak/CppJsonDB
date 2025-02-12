#ifndef VIEW_ALL_DATABASE_PERMISSIONS_COMMAND_H
#define VIEW_ALL_DATABASE_PERMISSIONS_COMMAND_H

#include "commands/command.h"
#include "databaseManager/databaseManager.h"
#include <iostream>
#include <vector>
#include <string>
#include "logManager/logManager.h"
class ViewAllDatabasePermissionsCommand : public Command {
public:
    ViewAllDatabasePermissionsCommand(std::string& currentUser);
    virtual void execute(const std::vector<std::string>& args) override;

private:
    std::string& currentUser;
};

#endif 
