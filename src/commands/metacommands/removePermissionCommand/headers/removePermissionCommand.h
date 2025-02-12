#ifndef REMOVE_PERMISSION_COMMAND_H
#define REMOVE_PERMISSION_COMMAND_H

#include "commands/command.h"
#include "databaseManager/databaseManager.h"
#include <string>
#include "checks.h"
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <iostream>
#include "logManager/logManager.h"

class RemovePermissionCommand : public Command {
public:
    RemovePermissionCommand(std::string& currentUser);

    void execute(const std::vector<std::string>& args) override;

private:
    std::string& currentUser;
};

#endif
