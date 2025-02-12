#include "headers/updateCommand.h"
#include "lockManager/lockManager.h"
#include "logManager/logManager.h"
#include "sqlManager/sqlManager.h"
#include "databaseManager/databaseManager.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include "utils.h"

UpdateCommand::UpdateCommand(std::string& currentUser, std::string& currentDatabase) : currentUser(currentUser), currentDatabase(currentDatabase) {}

bool UpdateCommand::checkPermissions() {
    INIT_DB_MANAGER;
    if (IS_LOCKED("", LockFileType::DATABASE_MANAGER)){
        DISPLAY_MESSAGE("database config file is currently locked... try again later");
        return false;
    }
    LOCK("", LockFileType::DATABASE_MANAGER, currentUser, "UPDATE");
    if (!dbManager.isOwner(currentDatabase, currentUser) &&
        !dbManager.checkUserPermission(currentDatabase, currentUser, Permission::ADD_TO_TABLE)) {
        DISPLAY_MESSAGE("Current user is neither the owner nor has modify permission for database '" + currentDatabase + "'.");
        UNLOCK("", LockFileType::DATABASE_MANAGER);
        return false;
    }
    UNLOCK("", LockFileType::DATABASE_MANAGER);
    return true;
}

void UpdateCommand::execute(const std::vector<std::string>& args) {
    INIT_SQL_MANAGER;

    LockFileType LockFileTypeToUse = LockFileType::DATABASE;
    std::vector<std::string> modifiedArgs = args;

    if (!checkPermissions()) {
        return;
    }

    if (args.size() < 2) {
        std::cerr << "Error: Invalid arguments for UPDATE command." << std::endl;
        return;
    }

    auto parsedArgs = parseArgsForUpdate(modifiedArgs);
    if (IS_LOCKED(currentDatabase, LockFileTypeToUse)){
        DISPLAY_MESSAGE(currentDatabase + " is currently locked... try again later");
        return;
    }

    LOCK(currentDatabase, LockFileTypeToUse, currentUser, "UPDATE");
    if (!sqlManager.updateRecords(parsedArgs, currentDatabase)) {
        std::cerr << "Error: Failed to update records." << std::endl;
    }
    UNLOCK(currentDatabase, LockFileTypeToUse);
}

std::map<std::string, std::map<std::string, std::vector<std::string>>> UpdateCommand::parseArgsForUpdate(std::vector<std::string>& args) {
    std::map<std::string, std::map<std::string, std::vector<std::string>>> tableColumnMap;

    if (args.size() < 3) {
        std::cerr << "Error: Insufficient arguments for UPDATE command." << std::endl;
        return tableColumnMap;
    }

    std::string tableName = args[0];
    tableColumnMap[tableName] = {};

    if (args[1] != "SET") {
        std::cerr << "Error: Invalid UPDATE command format." << std::endl;
        return tableColumnMap;
    }

    size_t whereIndex = std::find(args.begin(), args.end(), "WHERE") - args.begin();

    std::vector<std::string> setClauses;
    if (whereIndex > 2 && whereIndex <= args.size()) {
        setClauses.assign(args.begin() + 2, args.begin() + whereIndex);
    }

    std::vector<std::string> whereClauses;
    if (whereIndex < args.size()) {
        whereClauses.assign(args.begin() + whereIndex + 1, args.end());
    }

    for (const auto& clause : setClauses) {
        size_t pos = clause.find('=');
        if (pos != std::string::npos) {
            std::string column = trim(clause.substr(0, pos));
            std::string value = trim(clause.substr(pos + 1));

            if (!value.empty() && (value.front() == '\'' || value.front() == '"') && value.back() == value.front()) {
                value = value.substr(1, value.size() - 2);
            }

            tableColumnMap[tableName]["SET"].push_back(column);
            tableColumnMap[tableName]["VALUES"].push_back(value);
        }
    }

    if (!whereClauses.empty()) {
        std::string whereClause;

        for (const auto& token : whereClauses) {
            whereClause += token + " ";
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
            std::string column = trim(whereClause.substr(0, pos));
            std::string value = trim(whereClause.substr(pos + symbolFound.size()));

            if (!value.empty() && (value.front() == '\'' || value.front() == '"') && value.back() == value.front()) {
                value = value.substr(1, value.size() - 2);
            }

            tableColumnMap[tableName]["WHERE"] = { column, symbolFound, value };
        }
    } else {
        tableColumnMap[tableName][""] = {};
    }

    return tableColumnMap;
}
