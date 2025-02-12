#ifndef SQL_MANAGER_H
#define SQL_MANAGER_H

#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <map>
#include <vector>
#include <regex>
#include "utils.h"
#include "selectVariables.h"

using json = nlohmann::json;
using ResultTableColumns = std::map<std::string, std::map<std::string, std::vector<std::string>>>;
class SQLManager {
public:
    static SQLManager& getInstance() {
        static SQLManager instance;
        return instance;
    }
    bool deleteRecords(std::map<std::string, std::map<std::string, std::vector<std::string>>>& parsedArgs, std::string& currentDatabase);
    bool isNumeric(const std::string& str);
    bool createDatabase(const std::string& databaseName);
    bool deleteDatabase(const std::string& databaseName);
    bool createTable(const std::string& databaseName, const std::string& tableName, const json& columns);
    bool addRow(const std::string& databaseName, const std::string& tableName, const nlohmann::json& row);
    bool deleteRow(const std::string& databaseName, const std::string& tableName, const std::string& primaryKey);
    bool databaseFileExists(const std::string& databaseName) const;
    bool insertIntoTable(const std::string& databaseName, const std::string& tableName, const std::map<std::string, std::string>& columnValueMap);
    json findTable(const json& dbContent, const std::string& tableName);
    std::vector<nlohmann::json> findRecords(const nlohmann::json& rows, const std::string& column, const std::string& op, const std::string& value);
    bool readDatabaseFile(const std::string& databaseName, json& dbContent);
    bool writeDatabaseFile(const std::string& databaseName, const json& dbContent);

    std::map<std::string, std::map<std::string, std::vector<std::string>>> getTableAndColumnFromDatabaseForJoins(const std::string& databaseName, const std::vector<Join>& joinData);
    ResultTableColumns getTableAndColumnFromDatabase(const std::string& databaseName, const std::map<std::string, std::vector<std::string>>& tableColumnMap);

    bool applyUpdates(nlohmann::json& rows, const std::map<std::string, std::string>& updates, const std::map<std::string, std::string>& whereConditions);
    bool updateRecords(const std::map<std::string, std::map<std::string, std::vector<std::string>>>& parsedArgs, const std::string& databaseName);
private:
    SQLManager() = default;
    SQLManager(const SQLManager&) = delete;
    SQLManager& operator=(const SQLManager&) = delete;

    json queryDatabase(const std::string& query);

    void parseWhereClause(const std::string& whereClause, std::string& whereColumn, std::string& whereValue, bool& whereIsString);
    json processJoin(const json& dbContent, const std::string& joinType, const std::string& joinTable);
    bool filterRow(const json& row, const std::string& whereColumn, const std::string& whereValue, bool whereIsString);
    json selectColumns(const json& row, const std::vector<std::string>& columns, const std::vector<std::string>& tableColumns);
    void displayResults(const json& result, const std::vector<std::string>& selectedColumns);
    void suggestClosestColumn(const json& table, const std::string& whereColumn);
    bool hasNonNullValues(const json& resultRow);
};

#define INIT_SQL_MANAGER SQLManager& sqlManager = SQLManager::getInstance();

#endif
