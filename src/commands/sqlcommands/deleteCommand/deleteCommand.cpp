#include "headers/deleteCommand.h"
#include "lockManager/lockManager.h"
#include "logManager/logManager.h"
#include "sqlManager/sqlManager.h"
#include "databaseManager/databaseManager.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <iostream> 
#include "utils.h"

DeleteCommand::DeleteCommand(std::string& currentUser, std::string& currentDatabase) : currentUser(currentUser), currentDatabase(currentDatabase) {}

std::map<std::string, std::map<std::string, std::vector<std::string>>> DeleteCommand::parseArgsForDeletion(std::vector<std::string>& args) {
    std::map<std::string, std::map<std::string, std::vector<std::string>>> tableColumnMap;

    if (args.size() < 2) {
        std::cerr << "Error: Insufficient arguments for DELETE command." << std::endl;
        return tableColumnMap;
    }

    if (args[0] != "FROM") {
        std::cerr << "Error: Invalid DELETE command format." << std::endl;
        return tableColumnMap;
    }

    std::string tableName = args[1];
    tableColumnMap[tableName] = {};
    if (args.size() > 2 && args[2] == "WHERE") {
        std::string column, arithmeticSymbol, value;
        std::string whereClause;

        for (size_t i = 3; i < args.size(); ++i) {
            whereClause += args[i] + " ";
        }

        if (!whereClause.empty()) {
            whereClause.pop_back();
        }

        const std::vector<std::string> possibleSymbols = {">=", "<=", ">", "<", "="};

        std::string symbolFound;
        size_t pos = std::string::npos;
        for (const auto& symbol : possibleSymbols) {
            pos = whereClause.find(symbol);
            if (pos != std::string::npos) {
                symbolFound = symbol;
                break;
            }
        }

        if (!symbolFound.empty()) {
            column = whereClause.substr(0, pos);
            value = whereClause.substr(pos + symbolFound.size());

            column.erase(0, column.find_first_not_of(" \t"));
            column.erase(column.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            if (!value.empty() && (value.front() == '\'' || value.front() == '"') &&
                value.back() == value.front()) {
                value = value.substr(1, value.size() - 2);
            }

            tableColumnMap[tableName]["WHERE"] = { column, symbolFound, value };
        }
    }

    return tableColumnMap;
}
bool DeleteCommand::checkPermissions() {
    INIT_DB_MANAGER;
    if (IS_LOCKED("", LockFileType::DATABASE_MANAGER)){
        DISPLAY_MESSAGE("database config file is currently locked... try again later");
        return false;
    }
    LOCK("", LockFileType::DATABASE_MANAGER, currentUser, "DELETE");
    if (!dbManager.isOwner(currentDatabase, currentUser) &&
        !dbManager.checkUserPermission(currentDatabase, currentUser, Permission::ADD_TO_TABLE)) {
        DISPLAY_MESSAGE("Current user is neither the owner nor has read permission for database '" + currentDatabase + "'.");
        UNLOCK("", LockFileType::DATABASE_MANAGER);
        return false;
    }
    UNLOCK("", LockFileType::DATABASE_MANAGER);
    return true;
}
void DeleteCommand::execute(const std::vector<std::string>& args) {
    INIT_SQL_MANAGER;

    LockFileType LockFileTypeToUse = LockFileType::DATABASE;
    std::vector<std::string> modifiedArgs = args;

    if (!checkPermissions()) {
        return;
    }

    if (args.size() < 2) {
        std::cerr << "Error: Invalid arguments for DELETE command." << std::endl;
        return;
    }

    auto parsedArgs = parseArgsForDeletion(modifiedArgs);
    if (IS_LOCKED(currentDatabase, LockFileTypeToUse)){
        DISPLAY_MESSAGE(currentDatabase + " is currently locked... try again later");
        return;
    }

    LOCK(currentDatabase, LockFileTypeToUse, currentUser, "DELETE");
    if (!sqlManager.deleteRecords(parsedArgs, currentDatabase)) {
        std::cerr << "Error: Failed to delete records." << std::endl;
    }
    UNLOCK(currentDatabase, LockFileTypeToUse);
}
