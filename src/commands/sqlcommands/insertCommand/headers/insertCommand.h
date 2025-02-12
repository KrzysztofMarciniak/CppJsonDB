#ifndef INSERT_COMMAND_H
#define INSERT_COMMAND_H

#include "commands/command.h"
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include "sqlManager/sqlManager.h"
#include "checks.h"
#include "logManager/logManager.h"
#include "databaseManager/databaseManager.h"
#include <map>

class InsertCommand : public Command {
public:
    InsertCommand(std::string& currentUser, std::string& currentDatabase);
    void execute(const std::vector<std::string>& args) override;
private:
    void parseColumns(const std::string& str, std::vector<std::string>& columns);
    void parseValues(const std::string& str, std::vector<std::string>& values);
    std::string trim(const std::string& str);
    std::string collectArguments(const std::vector<std::string>& args, size_t start, size_t end) const;
    std::string& currentUser;
    std::string& currentDatabase;
};
#endif
