#ifndef SELECT_COMMAND_H
#define SELECT_COMMAND_H

#include "commands/command.h"
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include "sqlManager/sqlManager.h"
#include "logManager/logManager.h"
#include "databaseManager/databaseManager.h"
#include <map>
#include "selectVariables.h"
#include <stdexcept>
#include <sstream>
#include <iterator>
#include <cctype>
#include "lockManager/lockManager.h"
#include <future>
#include <algorithm>

const std::string CYAN = "\033[96m";
const std::string RED = "\033[91m";
const std::string GREEN = "\033[92m";
const std::string YELLOW = "\033[93m";
const std::string BLUE = "\033[94m";
const std::string MAGENTA = "\033[95m";
const std::string RESET = "\033[0m";


class SelectCommand : public Command {

public:
    SelectCommand(std::string& currentUser, std::string& currentDatabase);
    void execute(const std::vector<std::string>& args);

private:

    std::string trim(const std::string& str);
    void removeWhiteSpaces(std::vector<Join>& joinData);

    std::map<std::string, std::map<std::string, std::vector<std::string>>> mergeResultMaps(
    const std::map<std::string, std::map<std::string, std::vector<std::string>>>& existingMap,
    const std::map<std::string, std::map<std::string, std::vector<std::string>>>& joinMap
);


    std::string randomColor();
    void printTable(const std::map<std::string, std::map<std::string, std::vector<std::string>>>& resultMap);
    std::string& currentUser;
    std::string& currentDatabase;
    std::vector<std::vector<std::string>> splitJoinsFromArgs(const std::vector<std::vector<std::string>>& joinParts);
    void removeUnusedTables(std::map<std::string, std::vector<std::string>>& tableColumnMap);
    void displayQueryDetails(const std::map<std::string, std::vector<std::string>>& tableColumnMap);
    std::string getJoinType(const std::vector<std::string>& joinStr);
    bool detectJoinsFromArgs(const std::vector<std::string>& args);
    std::vector<std::vector<std::string>> extractJoinsFromArgs(const std::vector<std::string>& args);
    void parseJoinDetails(const std::vector<std::vector<std::string>>& joinStrs, std::vector<Join>& joinData);

    std::string parseArgs(std::vector<std::string>& args, const std::string& keyword);
    std::vector<Join> joins;

    std::vector<std::string>::iterator FindInArgs(std::vector<std::string>& args, const std::string& stringToFind);
    std::vector<std::string>::iterator FindOrderByInArgs(std::vector<std::string>& args);
    bool checkPermissions();
    std::string parseArgsOrderBy(std::vector<std::string>& args);
    std::string parseArgsWhere(std::vector<std::string>& args);
    int parseArgsLimit(std::vector<std::string>& args);
    std::map<std::string, std::vector<std::string>> parseArgsTablesColumns(const std::vector<std::string>& args);

    std::map<std::string, std::map<std::string, std::vector<std::string>>>  combineJoinsAndResultMap(
    const std::map<std::string, std::map<std::string, std::vector<std::string>>>& resultMap,
    const std::map<std::string, std::map<std::string, std::vector<std::string>>>& joinResultMap,
    const std::vector<Join>& joinData
    );

void rightJoinFunction(
    std::map<std::string, std::map<std::string, std::vector<std::string>>>& resultMap,
    const std::map<std::string, std::map<std::string, std::vector<std::string>>>& joinResultMap,
    const Join& join
);

void leftJoinFunction(
    std::map<std::string, std::map<std::string, std::vector<std::string>>>& resultMap,
    const std::map<std::string, std::map<std::string, std::vector<std::string>>>& joinResultMap,
    const Join& join
);

    void innerFunction(
    std::map<std::string, std::map<std::string, std::vector<std::string>>>& resultMap,
    const std::map<std::string, std::map<std::string, std::vector<std::string>>>& joinResultMap,
    const Join& join
    );
};

#endif
