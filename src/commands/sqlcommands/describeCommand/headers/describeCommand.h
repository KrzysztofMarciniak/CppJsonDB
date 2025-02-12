#ifndef DESCRIBE_COMMAND_H
#define DESCRIBE_COMMAND_H

#include "commands/command.h"
#include <iostream>
#include "sqlManager/sqlManager.h"
#include "logManager/logManager.h"
#include "databaseManager/databaseManager.h"
#include "checks.h"
#include "lockManager/lockManager.h"

class DescribeCommand : public Command {
public:
    DescribeCommand(std::string& currentUser, std::string& currentDatabase);
    virtual void execute(const std::vector<std::string>& args) override;
private:
    std::string &currentUser;
    std::string &currentDatabase;
};

#endif
