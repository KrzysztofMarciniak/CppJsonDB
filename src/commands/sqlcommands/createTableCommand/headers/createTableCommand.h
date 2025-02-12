#ifndef CREATETABLE_COMMAND_H
#define CREATETABLE_COMMAND_H

#include "commands/command.h"
#include <iostream>
#include <regex>
#include "checks.h"
#include "databaseManager/databaseManager.h"
#include "sqlManager/sqlManager.h"
#include "lockManager/lockManager.h"

class CreateTableCommand : public Command {
public:
    CreateTableCommand(std::string &currentUser, std::string& currentDatabase);
    void execute(const std::vector<std::string>& args) override;
private:
    std::string& currentUser;
    std::string& currentDatabase;
    void parseTables(const std::string& args, std::vector<std::pair<std::string, std::vector<json>>>& tables);
};

#endif
