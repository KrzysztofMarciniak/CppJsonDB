#include "headers/insertCommand.h"
#include <regex>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "lockManager/lockManager.h"
#include <map>

InsertCommand::InsertCommand(std::string& currentUser, std::string& currentDatabase)
    : currentUser(currentUser), currentDatabase(currentDatabase) {}

void InsertCommand::execute(const std::vector<std::string>& args) {
    INIT_DB_MANAGER;
    checkUserLoggedIn(currentUser);
    checkDatabaseSelected(currentUser, currentDatabase);
    LockFileType LockFileTypeToUse = LockFileType::DATABASE;
    bool isOwner = IS_OWNER(currentDatabase, currentUser);
    bool hasAddToTablePermissions = CHECK_USER_PERMISSION(currentDatabase, currentUser, Permission::ADD_TO_TABLE);

    if (!isOwner && !hasAddToTablePermissions) {
        DISPLAY_MESSAGE("Current user is neither the owner nor has add to table permission for " + currentDatabase);
        return;
    }


    if (args.size() < 4) {
        std::cerr << "Error: Insufficient arguments for INSERT command." << std::endl;
        return;
    }

    std::string tableName = args[1];

    auto it = std::find(args.begin(), args.end(), "values");
    if (it == args.end() || it == args.begin()) {
        std::cerr << "Error: Missing 'values' keyword or columns not specified." << std::endl;
        return;
    }

    std::string columnsStr = collectArguments(args, 2, it - args.begin());

    std::string valuesStr = collectArguments(args, it - args.begin() + 1, args.size());

    std::cout << "Debug: columnsStr = " << columnsStr << std::endl;
    std::cout << "Debug: valuesStr = " << valuesStr << std::endl;

    std::vector<std::string> columns;
    std::vector<std::string> values;

    parseColumns(columnsStr, columns);
    parseValues(valuesStr, values);

    std::cout << "Debug: columns.size() = " << columns.size() << std::endl;
    std::cout << "Debug: values.size() = " << values.size() << std::endl;

    if (columns.size() != values.size()) {
        std::cerr << "Error: Mismatch between number of provided columns and values." << std::endl;
        return;
    }

    std::map<std::string, std::string> columnValueMap;
    for (size_t i = 0; i < columns.size(); ++i) {
        columnValueMap[columns[i]] = values[i];
    }

    std::cout << "Table: " << tableName << std::endl;
    std::cout << "Columns: ";
    for (const auto& col : columns) {
        std::cout << col << " ";
    }
    std::cout << std::endl;
    std::cout << "Values: ";
    for (const auto& val : values) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    INIT_SQL_MANAGER;
    if (IS_LOCKED(currentDatabase, LockFileTypeToUse)){
        DISPLAY_MESSAGE(currentDatabase + " is currently locked... try again later");
        return;
    }
    LOCK(currentDatabase, LockFileTypeToUse, currentUser, "INSERT");
    if (!sqlManager.insertIntoTable(currentDatabase, tableName, columnValueMap)) {
        std::cerr << "Error: Failed to insert values into table '" << tableName << "' in database '" << currentDatabase << "'." << std::endl;
    }
    UNLOCK(currentDatabase, LockFileTypeToUse);
}

void InsertCommand::parseColumns(const std::string& str, std::vector<std::string>& columns) {
    std::regex columnsRegex(R"(\s*\(\s*([^\)]+)\s*\)\s*)");
    std::smatch match;
    if (std::regex_search(str, match, columnsRegex)) {
        std::string columnsPart = match[1].str();
        std::istringstream iss(columnsPart);
        std::string column;
        while (std::getline(iss, column, ',')) {
            columns.push_back(trim(column));
        }
    } else {
        std::cerr << "Error: Invalid columns format." << std::endl;
    }
}

void InsertCommand::parseValues(const std::string& str, std::vector<std::string>& values) {
    std::regex valuesRegex(R"(\s*\(\s*([^\)]+)\s*\)\s*)");
    std::smatch match;
    if (std::regex_search(str, match, valuesRegex)) {
        std::string valuesPart = match[1].str();
        std::istringstream iss(valuesPart);
        std::string value;

        while (std::getline(iss, value, ',')) {
            value = trim(value);


            if (!value.empty() && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            }

            values.push_back(value);
        }
    } else {
        std::cerr << "Error: Invalid values format." << std::endl;
    }
}

std::string InsertCommand::trim(const std::string& str) {
    size_t start = str.find_first_not_of(' ');
    size_t end = str.find_last_not_of(' ');
    if (start == std::string::npos || end == std::string::npos) {
        return "";
    }

    std::string trimmed = str.substr(start, end - start + 1);


    if (!trimmed.empty() && trimmed.front() == '"' && trimmed.back() == '"') {
        trimmed = trimmed.substr(1, trimmed.length() - 2);
    }

    return trimmed;
}

std::string InsertCommand::collectArguments(const std::vector<std::string>& args, size_t start, size_t end) const {
    std::ostringstream oss;
    for (size_t i = start; i < end; ++i) {
        if (i != start) {
            oss << " ";
        }
        oss << args[i];
    }
    return oss.str();
}
