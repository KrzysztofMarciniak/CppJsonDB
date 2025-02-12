#include "headers/createTableCommand.h"

using json = nlohmann::json;

CreateTableCommand::CreateTableCommand(std::string& currentUser, std::string& currentDatabase)
    : currentUser(currentUser), currentDatabase(currentDatabase) {}

void trim(std::string& str) {
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    str = str.substr(first, (last - first + 1));
}

void CreateTableCommand::parseTables(const std::string& args, std::vector<std::pair<std::string, std::vector<json>>>& tables) {
    std::regex tableRegex(R"(([^,]+)\(([^)]+)\))");
    std::regex columnRegex(R"((\w+)\s+(\w+)(\s*\{[^}]*\})?)");
    auto tablesBegin = std::sregex_iterator(args.begin(), args.end(), tableRegex);
    auto tablesEnd = std::sregex_iterator();

    for (std::sregex_iterator i = tablesBegin; i != tablesEnd; ++i) {
        std::smatch match = *i;
        std::string tableName = match.str(1);
        trim(tableName);
        std::string columnsStr = match.str(2);

        std::vector<json> columns;
        auto columnsBegin = std::sregex_iterator(columnsStr.begin(), columnsStr.end(), columnRegex);
        auto columnsEnd = std::sregex_iterator();

        for (std::sregex_iterator j = columnsBegin; j != columnsEnd; ++j) {
            std::smatch colMatch = *j;
            std::string colName = colMatch.str(1);
            std::string colType = colMatch.str(2);
            std::string colPropsStr = colMatch.str(3);
            json column = {{"name", colName}, {"type", colType}, {"properties", json::array()}};

            if (!colPropsStr.empty()) {
                std::regex propsRegex(R"(\{([^}]*)\})");
                std::smatch propsMatch;
                if (std::regex_search(colPropsStr, propsMatch, propsRegex)) {
                    std::string props = propsMatch.str(1);
                    std::stringstream ss(props);
                    std::string prop;
                    while (std::getline(ss, prop, ',')) {
                        prop.erase(0, prop.find_first_not_of(" \t"));
                        prop.erase(prop.find_last_not_of(" \t") + 1);
                        if (prop == "PRIMARY KEY" || prop == "AUTO_INCREMENT" || prop == "UNIQUE") {
                            column["properties"].push_back(prop);
                        } else if (prop.find("FOREIGN KEY REFERENCES") != std::string::npos) {
                            std::regex fkRegex(R"(FOREIGN KEY REFERENCES (\w+) (\w+))");
                            std::smatch fkMatch;
                            if (std::regex_search(prop, fkMatch, fkRegex)) {
                                json references = {{"table", fkMatch.str(1)}, {"column", fkMatch.str(2)}};
                                column["properties"].push_back(references);
                            } else {
                                std::cerr << "Error: Invalid FOREIGN KEY properties for column '" << colName << "'." << std::endl;
                                return;
                            }
                        }
                    }
                }
            }
            columns.push_back(column);
        }
        tables.emplace_back(tableName, columns);
    }
}


void CreateTableCommand::execute(const std::vector<std::string>& args) {
    checkUserLoggedIn(currentUser);
    checkDatabaseSelected(currentUser, currentDatabase);
    INIT_DB_MANAGER;
    LockFileType LockFileTypeToUse = LockFileType::DATABASE;

    bool isOwner = IS_OWNER(currentDatabase, currentUser);
    bool hasWritePermission = CHECK_USER_PERMISSION(currentDatabase, currentUser, Permission::WRITE);

    if (!isOwner && !hasWritePermission) {
        DISPLAY_MESSAGE("Current user is neither the owner nor has write permission for " + currentDatabase);
        return;
    }

    if (args.empty()) {
        std::cerr << "Error: No table creation arguments provided." << std::endl;
        return;
    }

    if (IS_LOCKED(currentDatabase, LockFileTypeToUse)){
        DISPLAY_MESSAGE(currentDatabase + " is currently locked... try again later");
        return;
    }
    LOCK(currentDatabase, LockFileTypeToUse, currentUser, "CREATE TABLE");

    std::string argsStr;
    for (const auto& arg : args) {
        argsStr += arg + " ";
    }

    std::vector<std::pair<std::string, std::vector<json>>> tables;
    parseTables(argsStr, tables);
    INIT_SQL_MANAGER;
    for (const auto& table : tables) {
        const std::string& tableName = table.first;
        const std::vector<json>& columns = table.second;
        if (!sqlManager.createTable(currentDatabase, tableName, columns)) {
            std::cerr << "Error: Failed to create table '" << tableName << "' in database '" << currentDatabase << "'." << std::endl;
        }
    }
    UNLOCK(currentDatabase, LockFileTypeToUse);
}
