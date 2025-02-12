#include "headers/selectCommand.h"
#include <fmt/core.h>
#include <iomanip>
#include <cstdlib>

std::string SelectCommand::randomColor() {
    int colorCode = rand() % 6;
    switch (colorCode) {
        case 0: return RED;
        case 1: return GREEN;
        case 2: return YELLOW;
        case 3: return BLUE;
        case 4: return MAGENTA;
        default: return CYAN;
    }
}

void SelectCommand::printTable(const std::map<std::string, std::map<std::string, std::vector<std::string>>>& resultMap) {
    if (resultMap.empty()) return;

    std::map<std::string, std::map<std::string, std::vector<std::string>>> combinedMap;
    size_t maxRows = 0;

    for (const auto& tablePair : resultMap) {
        const std::string& tableName = tablePair.first;
        const auto& columnsMap = tablePair.second;
        for (const auto& columnPair : columnsMap) {
            const std::string& columnName = columnPair.first;
            const auto& values = columnPair.second;

            if (combinedMap[tableName][columnName].empty()) {
                combinedMap[tableName][columnName].resize(values.size(), "NULL");
            }

            for (size_t i = 0; i < values.size(); ++i) {
                if (combinedMap[tableName][columnName][i] == "NULL") {
                    combinedMap[tableName][columnName][i] = values[i];
                } else {
                    combinedMap[tableName][columnName][i] += ", " + values[i];
                }
            }

            maxRows = std::max(maxRows, values.size());
        }
    }

    for (const auto& tablePair : combinedMap) {
        const std::string& tableName = tablePair.first;
        const auto& columnsMap = tablePair.second;

        if (columnsMap.empty()) continue;

        std::string tableColor = randomColor();
        std::cout << "Table: " << tableColor << tableName << RESET << std::endl;

        std::map<std::string, size_t> columnWidths;
        for (const auto& columnPair : columnsMap) {
            const auto& columnName = columnPair.first;
            columnWidths[columnName] = columnName.size();
            for (const auto& value : columnPair.second) {
                columnWidths[columnName] = std::max(columnWidths[columnName], value.size());
            }
        }

        std::cout << "+";
        for (const auto& column : columnsMap) {
            std::cout << std::string(columnWidths[column.first] + 2, '-') << "+";
        }
        std::cout << std::endl;

        std::cout << "|";
        for (const auto& column : columnsMap) {
            std::string columnColor = randomColor();
            std::cout << columnColor << std::setw(columnWidths[column.first] + 1) << std::left << column.first << RESET << " |";
        }
        std::cout << std::endl;

        std::cout << "+";
        for (const auto& column : columnsMap) {
            std::cout << std::string(columnWidths[column.first] + 2, '-') << "+";
        }
        std::cout << std::endl;

        for (size_t row = 0; row < maxRows; ++row) {
            std::cout << "|";
            for (const auto& column : columnsMap) {
                if (row < column.second.size()) {
                    std::cout << std::setw(columnWidths[column.first] + 1) << std::left << column.second[row] << " |";
                } else {
                    std::cout << std::setw(columnWidths[column.first] + 1) << std::left << "NULL" << " |";
                }
            }
            std::cout << std::endl;
        }

        std::cout << "+";
        for (const auto& column : columnsMap) {
            std::cout << std::string(columnWidths[column.first] + 2, '-') << "+";
        }
        std::cout << std::endl;
    }
}


SelectCommand::SelectCommand(std::string& currentUser, std::string& currentDatabase)
    : currentUser(currentUser), currentDatabase(currentDatabase) {}

std::vector<std::string>::iterator SelectCommand::FindInArgs(std::vector<std::string>& args, const std::string& stringToFind) {
    auto it = std::find(args.begin(), args.end(), stringToFind);
    return it;
}

std::vector<std::string>::iterator SelectCommand::FindOrderByInArgs(std::vector<std::string>& args) {
    auto it = FindInArgs(args, "ORDER");
    if (it != args.end() && std::distance(it, args.end()) > 1 && *(it + 1) == "BY") {
        return it;
    }
    return args.end();
}


bool SelectCommand::checkPermissions() {
    INIT_DB_MANAGER;
    if (!dbManager.isOwner(currentDatabase, currentUser) &&
        !dbManager.checkUserPermission(currentDatabase, currentUser, Permission::READ)) {
        DISPLAY_MESSAGE("Current user is neither the owner nor has read permission for database '" + currentDatabase + "'.");
        return false;
    }
    return true;
}
std::string SelectCommand::trim(const std::string& str) {
    auto start = str.find_first_not_of(' ');
    auto end = str.find_last_not_of(' ');
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}
void SelectCommand::removeWhiteSpaces(std::vector<Join>& joinData) {
    for (auto& join : joinData) {
        join.table = trim(join.table);
        join.joinType = trim(join.joinType);
        join.condition.leftTable = trim(join.condition.leftTable);
        join.condition.leftColumn = trim(join.condition.leftColumn);
        join.condition.rightTable = trim(join.condition.rightTable);
        join.condition.rightColumn = trim(join.condition.rightColumn);
    }
}

void SelectCommand::execute(const std::vector<std::string>& args) {
    INIT_SQL_MANAGER;
    std::vector<Join> joinData;
    bool joinsExist = false;
    LockFileType LockFileTypeToUse = LockFileType::DATABASE;
    std::vector<std::string> modifiedArgs = args;

    if (!checkPermissions()) {
        return;
    }

    if (args.size() < 3) {
        std::cerr << "Error: Invalid arguments for SELECT command." << std::endl;
        return;
    }


    if (detectJoinsFromArgs(modifiedArgs)) {
        std::vector<std::vector<std::string>> joinStrs = extractJoinsFromArgs(modifiedArgs);
        parseJoinDetails(joinStrs, joinData);
        removeWhiteSpaces(joinData);
        joinsExist = true;
    }


    //std::string whereClause = parseArgsWhere(modifiedArgs);
    //std::string orderByClause = parseArgsOrderBy(modifiedArgs);
    //int limit = parseArgsLimit(modifiedArgs);

    std::map<std::string, std::vector<std::string>> tableColumnMap = parseArgsTablesColumns(modifiedArgs);
    removeUnusedTables(tableColumnMap);

    if (IS_LOCKED(currentDatabase, LockFileTypeToUse)){
        DISPLAY_MESSAGE(currentDatabase + " is currently locked... try again later");
        return;
    }
    LOCK(currentDatabase, LockFileTypeToUse, currentUser, "SELECT");

    std::map<std::string, std::map<std::string, std::vector<std::string>>> resultMap = sqlManager.getTableAndColumnFromDatabase(currentDatabase, tableColumnMap);
    if (joinsExist) {
        std::map<std::string, std::map<std::string, std::vector<std::string>>> joinResultMap = sqlManager.getTableAndColumnFromDatabaseForJoins(currentDatabase, joinData);
        resultMap = combineJoinsAndResultMap(resultMap, joinResultMap, joinData);
    }
    printTable(resultMap);

    UNLOCK(currentDatabase, LockFileTypeToUse);
}

int SelectCommand::parseArgsLimit(std::vector<std::string>& args) {
    auto it = std::find(args.begin(), args.end(), "LIMIT");
    if (it != args.end() && it + 1 != args.end()) {
        try {
            int limit = std::stoi(*(it + 1));
            if (limit == 0) {
                return -1;
            }
            args.erase(it, it + 2);
            return limit;
        } catch (const std::invalid_argument& e) {
            return -1;
        }
    }
    return -1;
}




void removeFromArgs(std::vector<std::string>& args, std::vector<std::string>::iterator start, std::vector<std::string>::iterator end) {
    args.erase(start, end);
}

std::map<std::string, std::vector<std::string>> SelectCommand::parseArgsTablesColumns(const std::vector<std::string>& args) {
    std::map<std::string, std::vector<std::string>> tableColumnMap;
    std::vector<std::string> tables;
    std::vector<std::string> columnsBeforeFrom;
    bool fromSeen = false;
    std::string firstTableAfterFrom;

    for (const auto& arg : args) {
        if (arg == "FROM") {
            fromSeen = true;
            continue;
        }

        if (fromSeen) {
            if (arg.find('.') != std::string::npos) {
                continue;
            } else {
                if (tableColumnMap.find(arg) == tableColumnMap.end()) {
                    tableColumnMap[arg] = {};
                }
                tables.push_back(arg);
                if (firstTableAfterFrom.empty()) {
                    firstTableAfterFrom = arg;
                }
            }
        } else {
            if (arg.find('.') != std::string::npos) {
                std::string table = arg.substr(0, arg.find('.'));
                std::string column = arg.substr(arg.find('.') + 1);

                if (tableColumnMap.find(table) == tableColumnMap.end()) {
                    tableColumnMap[table] = {};
                }
                auto& columns = tableColumnMap[table];
                if (std::find(columns.begin(), columns.end(), column) == columns.end()) {
                    columns.push_back(column);
                }
            } else {
                columnsBeforeFrom.push_back(arg);
            }
        }
    }

    if (!columnsBeforeFrom.empty() && !firstTableAfterFrom.empty()) {
        auto& columns = tableColumnMap[firstTableAfterFrom];
        for (const auto& column : columnsBeforeFrom) {
            if (std::find(columns.begin(), columns.end(), column) == columns.end()) {
                columns.push_back(column);
            }
        }
    } else if (!columnsBeforeFrom.empty() && firstTableAfterFrom.empty()) {
        std::cerr << "Error: No table specified after FROM to assign columns specified before FROM." << std::endl;
    }
    return tableColumnMap;
}


