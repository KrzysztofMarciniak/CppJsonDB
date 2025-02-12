#ifndef ADD_PERMISSION_COMMAND_H
#define ADD_PERMISSION_COMMAND_H

#include "commands/command.h"
#include "databaseManager/databaseManager.h"
#include <string>
#include <vector>
#include "logManager/logManager.h"
#include "lockManager/lockManager.h"
#include "checks.h"
class AddPermissionCommand : public Command {
public:
    AddPermissionCommand(std::string& currentUser);
    void execute(const std::vector<std::string>& args) override;

private:
    std::string& currentUser;
};

#endif
