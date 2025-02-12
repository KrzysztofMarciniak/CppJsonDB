#include "sqlManager/sqlManager.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <map>
#include <string>
#include <tuple>
json SQLManager::findTable(const json& dbContent, const std::string& tableName) {
    auto& tables = dbContent["tables"];
    auto tableIt = std::find_if(tables.begin(), tables.end(), [&tableName](const json& table) {
        return table["table_name"] == tableName;
    });
    return (tableIt != tables.end()) ? *tableIt : json{};
}


bool SQLManager::insertIntoTable(const std::string& databaseName, const std::string& tableName, const std::map<std::string, std::string>& columnValueMap) {
    json dbContent;
    if (!readDatabaseFile(databaseName, dbContent)) {
        std::cerr << "Error: Database '" << databaseName << "' does not exist." << std::endl;
        return false;
    }

    json* targetTable = nullptr;
    for (auto& table : dbContent["tables"]) {
        if (table["table_name"] == tableName) {
            targetTable = &table;
            break;
        }
    }
    if (!targetTable) {
        std::cerr << "Error: Table '" << tableName << "' does not exist in database '" << databaseName << "'." << std::endl;
        return false;
    }

    const json& tableColumns = (*targetTable)["columns"];
    json& tableRows = (*targetTable)["rows"];

    std::string autoIncrementColumn;
    for (const auto& column : tableColumns) {
        if (std::find(column["properties"].begin(), column["properties"].end(), "AUTO_INCREMENT") != column["properties"].end()) {
            autoIncrementColumn = column["name"];
            break;
        }
    }

    json newRow;

    if (!autoIncrementColumn.empty()) {
        int nextId = 1;
        for (const auto& row : tableRows) {
            if (row.contains(autoIncrementColumn)) {
                int currentId = row[autoIncrementColumn].get<int>();
                if (currentId >= nextId) {
                    nextId = currentId + 1;
                }
            }
        }
        newRow[autoIncrementColumn] = nextId;
    }

    for (const auto& [columnName, value] : columnValueMap) {
        bool columnFound = false;
        for (const auto& column : tableColumns) {
            if (column["name"] == columnName) {
                columnFound = true;
                std::string columnType = column["type"];
                auto columnProperties = column.contains("properties") ? column["properties"] : json::array();

                for (const auto& row : tableRows) {
                    if (row.contains(columnName) && row[columnName] == value) {
                        for (const auto& property : columnProperties) {
                            if (property == "UNIQUE") {
                                std::cerr << "Error: Unique constraint violation. Value '"
                                          << value << "' already exists for column '"
                                          << columnName << "'." << std::endl;
                                return false;
                            }
                        }
                    }
                }

                if (columnType == "int") {
                    try {
                        newRow[columnName] = std::stoi(value);
                    } catch (const std::exception& e) {
                        std::cerr << "Error: Invalid integer value '" << value << "' for column '" << columnName << "'." << std::endl;
                        return false;
                    }
                } else if (columnType == "string") {
                    newRow[columnName] = value;
                } else {
                    std::cerr << "Error: Unsupported column type '" << columnType << "' for column '" << columnName << "'." << std::endl;
                    return false;
                }

                if (std::find(columnProperties.begin(), columnProperties.end(), "FOREIGN KEY") != columnProperties.end()) {
                    auto references = std::find_if(columnProperties.begin(), columnProperties.end(), [](const json& prop) {
                        return prop.is_object() && prop.contains("table") && prop.contains("column");
                    });

                    if (references != columnProperties.end()) {
                        std::string referencedTable = (*references)["table"];
                        std::string referencedColumn = (*references)["column"];
                        bool referenceExists = false;

                        for (const auto& refTable : dbContent["tables"]) {
                            if (refTable["table_name"] == referencedTable) {
                                for (const auto& refRow : refTable["rows"]) {
                                    if (refRow.contains(referencedColumn) && refRow[referencedColumn] == newRow[columnName]) {
                                        referenceExists = true;
                                        break;
                                    }
                                }
                                if (referenceExists) break;
                            }
                        }

                        if (!referenceExists) {
                            std::cerr << "Error: Foreign key constraint violation. Value '"
                                      << value << "' does not exist in column '"
                                      << referencedColumn << "' of table '"
                                      << referencedTable << "'." << std::endl;
                            return false;
                        }
                    }
                }

                break;
            }
        }

        if (!columnFound) {
            std::cerr << "Error: Column '" << columnName << "' does not exist in table '" << tableName << "'." << std::endl;
            return false;
        }
    }

    tableRows.push_back(newRow);

    if (!writeDatabaseFile(databaseName, dbContent)) {
        std::cerr << "Error: Failed to write to database file for database '" << databaseName << "'." << std::endl;
        return false;
    }

    return true;
}




bool SQLManager::deleteDatabase(const std::string& databaseName) {
    std::string dbFilePath = "data/databases/" + databaseName + ".db.json";
    if (std::filesystem::exists(dbFilePath)) {
        if (std::filesystem::remove(dbFilePath)) {
            return true;
        } else {
            std::cerr << "Error: Unable to delete the database file: " << dbFilePath << "\n";
            return false;
        }
    } else {
        std::cerr << "Error: No database found at " << dbFilePath << "\n";
        return false;
    }
}

bool SQLManager::createDatabase(const std::string& databaseName) {
    std::string dbFilePath = "data/databases/" + databaseName + ".db.json";
    if (std::filesystem::exists(dbFilePath)) {
        std::cerr << "Error: Database file '" << dbFilePath << "' already exists." << std::endl;
        return false;
    }

    nlohmann::json dbContent = {
        {"database_name", databaseName},
        {"tables", nlohmann::json::array()}
    };

    return writeDatabaseFile(databaseName, dbContent);
}


bool SQLManager::createTable(const std::string& databaseName, const std::string& tableName, const json& columns) {
    json dbContent;
    if (!readDatabaseFile(databaseName, dbContent)) {
        std::cerr << "Error: Database '" << databaseName << "' does not exist." << std::endl;
        return false;
    }

    for (const auto& table : dbContent["tables"]) {
        if (table["table_name"] == tableName) {
            std::cerr << "Error: Table '" << tableName << "' already exists in database '" << databaseName << "'." << std::endl;
            return false;
        }
    }

    json newTable = {
        {"table_name", tableName},
        {"columns", json::array()},
        {"rows", json::array()}
    };

    for (const auto& col : columns) {
        json column = {
            {"name", col["name"].get<std::string>()},
            {"type", col["type"].get<std::string>()},
            {"properties", json::array()}
        };

        bool hasAutoIncrement = false;
        bool hasPrimaryKey = false;

        if (col.contains("properties")) {
            const auto& properties = col["properties"];
            for (const auto& prop : properties) {
                if (prop.is_string()) {
                    std::string propStr = prop.get<std::string>();
                    if (propStr == "AUTO_INCREMENT") {
                        hasAutoIncrement = true;
                    } else if (propStr == "PRIMARY KEY") {
                        hasPrimaryKey = true;
                    }
                    column["properties"].push_back(propStr);
                } else if (prop.is_object() && prop.contains("table") && prop.contains("column")) {
                    column["properties"].push_back("FOREIGN KEY");
                    column["properties"].push_back(prop);
                }
            }
        }

        std::string type = column["type"];
        if (type != "int" && type != "string" && type != "date") {
            std::cerr << "Error: Invalid column type '" << type << "' for column '" << column["name"] << "'." << std::endl;
            return false;
        }

        if ((hasAutoIncrement || hasPrimaryKey) && type != "int") {
            std::cerr << "Error: 'AUTO_INCREMENT' and 'PRIMARY KEY' can only be applied to columns of type 'int'." << std::endl;
            return false;
        }

        for (const auto& prop : column["properties"]) {
            if (prop == "FOREIGN KEY" && type != "int") {
                std::cerr << "Error: Foreign key relations can only reference columns of type 'int'." << std::endl;
                return false;
            }
        }

        newTable["columns"].push_back(column);
    }

    dbContent["tables"].push_back(newTable);

    if (!writeDatabaseFile(databaseName, dbContent)) {
        std::cerr << "Error: Failed to write updated database content to file." << std::endl;
        return false;
    }

    std::cout << "Table '" << tableName << "' created successfully in database '" << databaseName << "'." << std::endl;
    return true;
}



bool SQLManager::addRow(const std::string& databaseName, const std::string& tableName, const nlohmann::json& row) {
    nlohmann::json dbContent;
    if (!readDatabaseFile(databaseName, dbContent)) {
        std::cerr << "Error: Database '" << databaseName << "' does not exist." << std::endl;
        return false;
    }

    for (auto& table : dbContent["tables"]) {
        if (table["table_name"] == tableName) {
            for (const auto& column : table["columns"]) {
                if (column.contains("foreignKey")) {
                    std::string refTable = column["foreignKey"]["table"];
                    std::string refColumn = column["foreignKey"]["column"];
                    bool refExists = false;

                    for (const auto& refTableData : dbContent["tables"]) {
                        if (refTableData["table_name"] == refTable) {
                            for (const auto& refRow : refTableData["rows"]) {
                                if (refRow[refColumn] == row[column["name"]]) {
                                    refExists = true;
                                    break;
                                }
                            }
                        }
                        if (refExists) break;
                    }

                    if (!refExists) {
                        std::cerr << "Error: Foreign key constraint violation for table '" << tableName << "', column '" << column["name"] << "'." << std::endl;
                        return false;
                    }
                }
            }

            table["rows"].push_back(row);
            return writeDatabaseFile(databaseName, dbContent);
        }
    }

    std::cerr << "Error: Table '" << tableName << "' does not exist in database '" << databaseName << "'." << std::endl;
    return false;
}

ResultTableColumns SQLManager::getTableAndColumnFromDatabase(const std::string& databaseName, const std::map<std::string, std::vector<std::string>>& tableColumnMap) {
    ResultTableColumns resultTableColumns;
    json dbContent;

    if (!readDatabaseFile(databaseName, dbContent)) {
        std::cerr << "Error: Could not read database file." << std::endl;
        return resultTableColumns;
    }

    for (const auto& table : dbContent["tables"]) {
        std::string tableName = table["table_name"];
        const auto& columns = table["columns"];
        const auto& rows = table["rows"];

        if (tableColumnMap.find(tableName) != tableColumnMap.end()) {
            auto& columnMap = resultTableColumns[tableName];

            for (const auto& column : tableColumnMap.at(tableName)) {
                columnMap[column] = {};
            }

            for (const auto& row : rows) {
                for (const auto& columnPair : columnMap) {
                    const std::string& columnName = columnPair.first;
                    if (row.contains(columnName)) {
                        columnMap[columnName].push_back(row[columnName].dump());
                    } else {
                        columnMap[columnName].push_back("NULL");
                    }
                }
            }
        }
    }

    return resultTableColumns;
}



bool SQLManager::deleteRow(const std::string& databaseName, const std::string& tableName, const std::string& primaryKey) {
    nlohmann::json dbContent;
    if (!readDatabaseFile(databaseName, dbContent)) {
        std::cerr << "Error: Database '" << databaseName << "' does not exist." << std::endl;
        return false;
    }

    for (auto& table : dbContent["tables"]) {
        if (table["table_name"] == tableName) {
            for (auto it = table["rows"].begin(); it != table["rows"].end(); ++it) {
                if ((*it)["id"] == primaryKey) { 
                    table["rows"].erase(it);
                    return writeDatabaseFile(databaseName, dbContent);
                }
            }
        }
    }

    std::cerr << "Error: Row with primary key '" << primaryKey << "' does not exist in table '" << tableName << "' of database '" << databaseName << "'." << std::endl;
    return false;
}

bool SQLManager::readDatabaseFile(const std::string& databaseName, nlohmann::json& dbContent) {
    std::string dbFilePath = "data/databases/" + databaseName + ".db.json";
    std::ifstream inputFile(dbFilePath);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Failed to open JSON file '" << dbFilePath << "'." << std::endl;
        return false;
    }

    try {
        inputFile >> dbContent;
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to parse JSON from file '" << dbFilePath << "': " << e.what() << std::endl;
        inputFile.close();
        return false;
    }

    inputFile.close();
    return true;
}

bool SQLManager::writeDatabaseFile(const std::string& databaseName, const nlohmann::json& dbContent) {
    std::string dbFilePath = "data/databases/" + databaseName + ".db.json";
    std::ofstream outputFile(dbFilePath);

    if (!outputFile.is_open()) {
        std::cerr << "Error: Failed to open JSON file '" << dbFilePath << "' for writing." << std::endl;
        return false;
    }

    try {
        outputFile << std::setw(4) << dbContent << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to write JSON to file. Exception: " << e.what() << std::endl;
        outputFile.close();
        return false;
    }

    outputFile.close();
    return true;
}

bool SQLManager::databaseFileExists(const std::string& databaseName) const {
    std::string dbFilePath = "data/databases/" + databaseName + ".db.json";
    return std::filesystem::exists(dbFilePath);
}

bool SQLManager::isNumeric(const std::string& str) {
    try {
        std::stod(str);
    } catch (std::invalid_argument&) {
        return false;
    } catch (std::out_of_range&) {
        return false;
    }
    return true;
}

bool SQLManager::deleteRecords(std::map<std::string, std::map<std::string, std::vector<std::string>>>& parsedArgs, std::string& currentDatabase) {
    nlohmann::json dbContent;

    if (!readDatabaseFile(currentDatabase, dbContent)) {
        std::cerr << "Error: Failed to read the database file." << std::endl;
        return false;
    }

    for (auto& [tableName, conditions] : parsedArgs) {

        for (auto& table : dbContent["tables"]) {
            if (table["table_name"] == tableName) {

                auto& rows = table["rows"];

                if (conditions.contains("WHERE")) {
                    const auto& condition = conditions.at("WHERE");

                    if (condition.size() == 3) {
                        std::string column = condition[0];
                        std::string op = condition[1];
                        std::string value = condition[2];

                        std::cout << "Applying WHERE condition: " << column << " " << op << " " << value << std::endl;

                        std::vector<nlohmann::json> remainingRows;

                        for (const auto& row : rows) {
                            bool match = false;

                            if (row.contains(column)) {
                                auto columnValue = row[column];

                                if (isNumeric(value) && columnValue.is_number()) {
                                    double columnValueNum = columnValue.get<double>();
                                    double compareValueNum = std::stod(value);

                                    if (op == "=") {
                                        match = (columnValueNum == compareValueNum);
                                    } else if (op == "<") {
                                        match = (columnValueNum < compareValueNum);
                                    } else if (op == ">") {
                                        match = (columnValueNum > compareValueNum);
                                    } else if (op == "<=") {
                                        match = (columnValueNum <= compareValueNum);
                                    } else if (op == ">=") {
                                        match = (columnValueNum >= compareValueNum);
                                    }
                                } else if (columnValue.is_string()) {
                                    std::string columnValueStr = columnValue.get<std::string>();

                                    if (op == "=") {
                                        match = (columnValueStr == value);
                                    } else if (op == "<") {
                                        match = (columnValueStr < value);
                                    } else if (op == ">") {
                                        match = (columnValueStr > value);
                                    } else if (op == "<=") {
                                        match = (columnValueStr <= value);
                                    } else if (op == ">=") {
                                        match = (columnValueStr >= value);
                                    }
                                }
                            }

                            if (!match) {
                                remainingRows.push_back(row);  
                            }
                        }

                        rows = remainingRows;

                    } else {
                        std::cerr << "Error: Invalid WHERE clause format." << std::endl;
                        return false;
                    }
                } else {
                    rows.clear();
                }

                break;
            }
        }
    }
    if (!writeDatabaseFile(currentDatabase, dbContent)) {
        std::cerr << "Error: Failed to write the database file." << std::endl;
        return false;
    }

    return true;
}

bool SQLManager::updateRecords(const std::map<std::string, std::map<std::string, std::vector<std::string>>>& parsedArgs, const std::string& databaseName) {
    nlohmann::json dbContent;

    if (!readDatabaseFile(databaseName, dbContent)) {
        std::cerr << "Error: Failed to read the database file." << std::endl;
        return false;
    }

    bool success = true;

    for (auto& [tableName, conditions] : parsedArgs) {
        auto tableIt = std::find_if(dbContent["tables"].begin(), dbContent["tables"].end(), [&tableName](const nlohmann::json& table) {
            return table["table_name"] == tableName;
        });

        if (tableIt != dbContent["tables"].end()) {
            auto& table = *tableIt;
            auto& rows = table["rows"];
            auto& columns = table["columns"];

            std::map<std::string, std::string> updates;
            if (conditions.contains("SET")) {
                const auto& setConditions = conditions.at("SET");
                if (setConditions.size() != 1 || conditions.at("VALUES").size() != 1) {
                    std::cerr << "Error: SET clause must have exactly one column and one value." << std::endl;
                    success = false;
                    continue;
                }

                std::string column = setConditions[0];
                std::string value = conditions.at("VALUES")[0];
                updates[column] = value;
            } else {
                std::cerr << "Error: No SET clause found." << std::endl;
                success = false;
                continue;
            }

            nlohmann::json updatedRows = nlohmann::json::array();
            if (conditions.contains("WHERE")) {
                const auto& whereConditions = conditions.at("WHERE");

                if (whereConditions.size() == 3 && whereConditions[1] == "=") {
                    std::string whereColumn = whereConditions[0];
                    std::string whereValue = whereConditions[2];

                    for (auto& row : rows) {
                        bool match = false;
                        auto columnIt = std::find_if(columns.begin(), columns.end(), [&whereColumn](const nlohmann::json& col) {
                            return col["name"] == whereColumn;
                        });

                        if (columnIt != columns.end()) {
                            if ((*columnIt)["type"] == "int") {
                                match = row[whereColumn] == std::stoi(whereValue);
                            } else if ((*columnIt)["type"] == "string") {
                                match = row[whereColumn] == whereValue;
                            }
                        }

                        if (match) {
                            for (const auto& [column, value] : updates) {
                                auto columnType = std::find_if(columns.begin(), columns.end(), [&column](const nlohmann::json& col) {
                                    return col["name"] == column;
                                });

                                if (columnType != columns.end()) {
                                    if ((*columnType)["type"] == "int") {
                                        row[column] = std::stoi(value);
                                    } else if ((*columnType)["type"] == "string") {
                                        row[column] = value;
                                    }
                                }
                            }
                        }
                        updatedRows.push_back(row);
                    }
                } else {
                    std::cerr << "Error: Invalid WHERE clause." << std::endl;
                    success = false;
                    continue;
                }
            } else {
                for (auto& row : rows) {
                    for (const auto& [column, value] : updates) {
                        auto columnType = std::find_if(columns.begin(), columns.end(), [&column](const nlohmann::json& col) {
                            return col["name"] == column;
                        });

                        if (columnType != columns.end()) {
                            if ((*columnType)["type"] == "int") {
                                row[column] = std::stoi(value);
                            } else if ((*columnType)["type"] == "string") {
                                row[column] = value;
                            }
                        }
                    }
                    updatedRows.push_back(row);
                }
            }

            table["rows"] = updatedRows;
            *tableIt = table;
        } else {
            std::cerr << "Error: Table '" << tableName << "' not found in the database." << std::endl;
            success = false;
        }
    }

    if (!writeDatabaseFile(databaseName, dbContent)) {
        std::cerr << "Error: Failed to write the updated database file." << std::endl;
        success = false;
    }

    return success;
}
