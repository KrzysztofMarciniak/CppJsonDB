#include "headers/selectCommand.h"

std::string SelectCommand::getJoinType(const std::vector<std::string>& joinStr) {
    std::string joinType = "UNKNOWN JOIN TYPE";

    std::unordered_set<std::string> validJoinTypes = {
        "INNER", "LEFT", "RIGHT", "FULL"
    };

    for (const auto& str : joinStr) {
        std::string upperStr = str;
        std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);

        if (validJoinTypes.count(upperStr) > 0) {
            joinType = upperStr + " JOIN";
            break;
        }
    }

    return joinType;
}

void SelectCommand::removeUnusedTables(std::map<std::string, std::vector<std::string>>& tableColumnMap) {
    for (auto it = tableColumnMap.begin(); it != tableColumnMap.end();) {
        if (it->second.empty()) {
            it = tableColumnMap.erase(it);
        } else {
            ++it;
        }
    }
}

void SelectCommand::displayQueryDetails(const std::map<std::string, std::vector<std::string>>& tableColumnMap) {
    std::cout << "Table Column Map:" << std::endl;
    for (const auto& tableColumns : tableColumnMap) {
        std::cout << "  - Table: " << tableColumns.first << std::endl;
        for (const auto& column : tableColumns.second) {
            std::cout << "    - Column: " << column << std::endl;
        }
    }
}

bool SelectCommand::detectJoinsFromArgs(const std::vector<std::string>& args) {
    std::vector<std::string> joinKeywords = {
        "JOIN",
        "INNER JOIN",
        "LEFT JOIN",
        "RIGHT JOIN",
        "FULL JOIN"
    };

    for (const auto& keyword : joinKeywords) {
        if (std::find(args.begin(), args.end(), keyword) != args.end()) {
            return true;
        }
    }

    return false;
}
std::vector<std::vector<std::string>> SelectCommand::extractJoinsFromArgs(const std::vector<std::string>& args) {
    std::vector<std::vector<std::string>> joinParts;
    std::vector<std::string> sqlKeywords = {"WHERE", "LIMIT", "ORDER"};
    bool moreThanOneJoin = false;

    auto it = args.begin();
    while (it != args.end()) {
        auto joinIt = std::find(it, args.end(), "JOIN");
        if (joinIt == args.end()) break;

        std::vector<std::string> joinPart;
        if (joinIt != args.begin()) {
            std::string prevToken = *(joinIt - 1);
            if (std::all_of(prevToken.begin(), prevToken.end(), ::isupper)) {
                joinPart.push_back(prevToken);
            }
        }

        joinPart.push_back("JOIN");

        auto onIt = std::find(joinIt + 1, args.end(), "ON");
        if (onIt == args.end()) break;

        auto nextJoinIt = std::find(joinIt + 1, args.end(), "JOIN");
        auto nextKeywordIt = std::find_if(onIt + 1, args.end(), [&](const std::string& token) {
            return std::find(sqlKeywords.begin(), sqlKeywords.end(), token) != sqlKeywords.end();
        });

        auto endIt = (nextJoinIt != args.end()) ? nextJoinIt : nextKeywordIt;

        for (auto jt = joinIt + 1; jt != endIt && jt != args.end(); ++jt) {
            joinPart.push_back(*jt);
        }

        joinParts.push_back(joinPart);

        if (nextJoinIt != args.end()) {
            moreThanOneJoin = true;
        }

        it = endIt;
    }

    if (moreThanOneJoin) {
        std::vector<std::vector<std::string>> splitJoinParts = splitJoinsFromArgs(joinParts);
        return splitJoinParts;
    } else {
        return joinParts;
    }
}
std::vector<std::vector<std::string>> SelectCommand::splitJoinsFromArgs(const std::vector<std::vector<std::string>>& joinParts) {
    std::vector<std::vector<std::string>> splitJoinParts;

    for (const auto& joinPart : joinParts) {
        std::vector<std::string> splitPart;
        std::string joinStr;
        for (const auto& token : joinPart) {
            if (token == "JOIN") {
                if (!joinStr.empty()) {
                    splitJoinParts.push_back({joinStr});
                }
                joinStr.clear();
            }
            joinStr += token + " ";
        }
        if (!joinStr.empty()) {
            splitJoinParts.push_back({joinStr});
        }
    }
    return splitJoinParts;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> parts;
    std::string part;
    for (char c : str) {
        if (c == delimiter) {
            parts.push_back(part);
            part.clear();
        } else {
            part += c;
        }
    }
    parts.push_back(part);
    return parts;
}



void SelectCommand::parseJoinDetails(const std::vector<std::vector<std::string>>& joinStrs, std::vector<Join>& joinData) {
    for (const auto& joinStr : joinStrs) {
        Join join;

        std::string joinType;
        if (joinStr.size() >= 2 && (joinStr[0] == "INNER" || joinStr[0] == "LEFT" || joinStr[0] == "RIGHT" || joinStr[0] == "FULL")) {
            joinType = joinStr[0] + " " + joinStr[1];
        } else {
            joinType = "INNER JOIN";
        }

        std::vector<std::string> tokens;
        for (const auto& str : joinStr) {
            std::vector<std::string> splitTokens = split(str, ' ');
            tokens.insert(tokens.end(), splitTokens.begin(), splitTokens.end());
        }

        auto joinIt = std::find(tokens.begin(), tokens.end(), "JOIN");
        if (joinIt == tokens.end() || (joinIt + 1) == tokens.end()) {
            std::cerr << "Error: Table name missing after JOIN keyword." << std::endl;
            continue;
        }
        join.table = *(joinIt + 1);

        auto onIt = std::find(tokens.begin(), tokens.end(), "ON");
        if (onIt == tokens.end() || (onIt + 1) == tokens.end()) {
            std::cerr << "Error: Missing or incomplete ON clause in JOIN specification." << std::endl;
            continue;
        }

        std::string conditionStr;
        for (auto it = onIt + 1; it != tokens.end(); ++it) {
            conditionStr += *it + " ";
        }

        auto conditionParts = split(conditionStr, '=');
        if (conditionParts.size() != 2) {
            std::cerr << "Error: JOIN condition should contain exactly one '=' character." << std::endl;
            continue;
        }

        auto leftParts = split(conditionParts[0], '.');
        auto rightParts = split(conditionParts[1], '.');
        if (leftParts.size() != 2 || rightParts.size() != 2) {
            std::cerr << "Error: Invalid format in JOIN condition. Expected format 'table.column = table.column'." << std::endl;
            continue;
        }

        JoinCondition joinCondition;
        joinCondition.leftTable = leftParts[0];
        joinCondition.leftColumn = leftParts[1];
        joinCondition.rightTable = rightParts[0];
        joinCondition.rightColumn = rightParts[1];

        join.condition = joinCondition;
        join.joinType = joinType;

        joinData.push_back(join);
    }
}

std::map<std::string, std::map<std::string, std::vector<std::string>>> SelectCommand::combineJoinsAndResultMap(
    const std::map<std::string, std::map<std::string, std::vector<std::string>>>& resultMap,
    const std::map<std::string, std::map<std::string, std::vector<std::string>>>& joinResultMap,
    const std::vector<Join>& joinData
) {
    std::map<std::string, std::map<std::string, std::vector<std::string>>> finalResult = resultMap;

    for (const auto& join : joinData) {
        if (join.joinType == "INNER JOIN") {
            innerFunction(finalResult, joinResultMap, join);
        } else if (join.joinType == "LEFT JOIN") {
            leftJoinFunction(finalResult, joinResultMap, join);
        } else if (join.joinType == "RIGHT JOIN") {
            rightJoinFunction(finalResult, joinResultMap, join);
        } else if (join.joinType == "FULL JOIN") {
            std::cout << "FULL JOIN NOT IMPLEMENTED...";
            return finalResult;
        }
    }

    for (const auto& [table, columns] : resultMap) {
        if (finalResult.find(table) == finalResult.end()) {
            finalResult[table] = columns;
        }
    }

    return finalResult;
}

void SelectCommand::innerFunction(
    std::map<std::string, std::map<std::string, std::vector<std::string>>>& resultMap,
    const std::map<std::string, std::map<std::string, std::vector<std::string>>>& joinResultMap,
    const Join& join
) {
    std::string leftTable = join.condition.leftTable;
    std::string leftColumn = join.condition.leftColumn;
    std::string rightTable = join.condition.rightTable;
    std::string rightColumn = join.condition.rightColumn;

    auto leftTableIt = resultMap.find(leftTable);
    auto rightTableIt = joinResultMap.find(rightTable);

    if (leftTableIt == resultMap.end() || rightTableIt == joinResultMap.end()) {
        std::cerr << "Error: One or both tables not found in result maps." << std::endl;
        return;
    }

    const auto& leftTableData = leftTableIt->second;
    const auto& rightTableData = rightTableIt->second;

    auto leftColumnIt = leftTableData.find(leftColumn);
    auto rightColumnIt = rightTableData.find(rightColumn);

    if (leftColumnIt == leftTableData.end() || rightColumnIt == rightTableData.end()) {
        std::cerr << "Error: One or both columns not found in result maps." << std::endl;
        return;
    }

    const auto& leftColumnValues = leftColumnIt->second;
    const auto& rightColumnValues = rightColumnIt->second;

    std::map<std::string, std::vector<std::string>> resultColumns;

    for (const auto& columnPair : leftTableData) {
        std::string colName = leftTable + "." + columnPair.first;
        resultColumns[colName] = {};
    }
    for (const auto& columnPair : rightTableData) {
        std::string colName = rightTable + "." + columnPair.first;
        resultColumns[colName] = {};
    }

    for (size_t i = 0; i < leftColumnValues.size(); ++i) {
        const std::string& leftValue = leftColumnValues[i];

        for (size_t j = 0; j < rightColumnValues.size(); ++j) {
            const std::string& rightValue = rightColumnValues[j];

            if (leftValue == rightValue) {
                for (const auto& columnPair : leftTableData) {
                    std::string colName = leftTable + "." + columnPair.first;
                    resultColumns[colName].push_back(columnPair.second[i]);
                }

                for (const auto& columnPair : rightTableData) {
                    std::string colName = rightTable + "." + columnPair.first;
                    resultColumns[colName].push_back(columnPair.second[j]);
                }
            }
        }
    }

    resultMap[leftTable] = resultColumns;
}


void SelectCommand::leftJoinFunction(
    std::map<std::string, std::map<std::string, std::vector<std::string>>>& resultMap,
    const std::map<std::string, std::map<std::string, std::vector<std::string>>>& joinResultMap,
    const Join& join
) {
    std::string leftTable = join.condition.leftTable;
    std::string leftColumn = join.condition.leftColumn;
    std::string rightTable = join.condition.rightTable;
    std::string rightColumn = join.condition.rightColumn;

    auto leftTableIt = resultMap.find(leftTable);
    auto rightTableIt = joinResultMap.find(rightTable);

    if (leftTableIt == resultMap.end() || rightTableIt == joinResultMap.end()) {
        std::cerr << "Error: One or both tables not found in result maps." << std::endl;
        return;
    }

    const auto& leftTableData = leftTableIt->second;
    const auto& rightTableData = rightTableIt->second;

    auto leftColumnIt = leftTableData.find(leftColumn);
    auto rightColumnIt = rightTableData.find(rightColumn);

    if (leftColumnIt == leftTableData.end() || rightColumnIt == rightTableData.end()) {
        std::cerr << "Error: One or both columns not found in result maps." << std::endl;
        return;
    }

    const auto& leftColumnValues = leftColumnIt->second;
    const auto& rightColumnValues = rightColumnIt->second;

    std::map<std::string, std::vector<std::string>> resultColumns;

    for (const auto& columnPair : leftTableData) {
        std::string colName = leftTable + "." + columnPair.first;
        resultColumns[colName] = {};
    }
    for (const auto& columnPair : rightTableData) {
        std::string colName = rightTable + "." + columnPair.first;
        resultColumns[colName] = {};
    }

    for (size_t i = 0; i < leftColumnValues.size(); ++i) {
        const std::string& leftValue = leftColumnValues[i];
        bool matchFound = false;

        for (size_t j = 0; j < rightColumnValues.size(); ++j) {
            const std::string& rightValue = rightColumnValues[j];

            if (leftValue == rightValue) {
                for (const auto& columnPair : leftTableData) {
                    std::string colName = leftTable + "." + columnPair.first;
                    resultColumns[colName].push_back(columnPair.second[i]);
                }

                for (const auto& columnPair : rightTableData) {
                    std::string colName = rightTable + "." + columnPair.first;
                    resultColumns[colName].push_back(columnPair.second[j]);
                }

                matchFound = true;
                break;
            }
        }

        if (!matchFound) {
            for (const auto& columnPair : leftTableData) {
                std::string colName = leftTable + "." + columnPair.first;
                resultColumns[colName].push_back(columnPair.second[i]);
            }
            for (const auto& columnPair : rightTableData) {
                std::string colName = rightTable + "." + columnPair.first;
                resultColumns[colName].push_back("NULL");
            }
        }
    }

    resultMap[leftTable + "_" + rightTable] = resultColumns;
}
void SelectCommand::rightJoinFunction(
    std::map<std::string, std::map<std::string, std::vector<std::string>>>& resultMap,
    const std::map<std::string, std::map<std::string, std::vector<std::string>>>& joinResultMap,
    const Join& join
) {
    std::string leftTable = join.condition.leftTable;
    std::string leftColumn = join.condition.leftColumn;
    std::string rightTable = join.condition.rightTable;
    std::string rightColumn = join.condition.rightColumn;

    auto leftTableIt = resultMap.find(leftTable);
    auto rightTableIt = joinResultMap.find(rightTable);

    if (leftTableIt == resultMap.end() || rightTableIt == joinResultMap.end()) {
        std::cerr << "Error: One or both tables not found in result maps." << std::endl;
        return;
    }

    const auto& leftTableData = leftTableIt->second;
    const auto& rightTableData = rightTableIt->second;

    auto leftColumnIt = leftTableData.find(leftColumn);
    auto rightColumnIt = rightTableData.find(rightColumn);

    if (leftColumnIt == leftTableData.end() || rightColumnIt == rightTableData.end()) {
        std::cerr << "Error: One or both columns not found in result maps." << std::endl;
        return;
    }

    const auto& leftColumnValues = leftColumnIt->second;
    const auto& rightColumnValues = rightColumnIt->second;

    std::map<std::string, std::vector<std::string>> resultColumns;

    for (const auto& columnPair : leftTableData) {
        std::string colName = leftTable + "." + columnPair.first;
        resultColumns[colName] = {};
    }
    for (const auto& columnPair : rightTableData) {
        std::string colName = rightTable + "." + columnPair.first;
        resultColumns[colName] = {};
    }

    for (size_t j = 0; j < rightColumnValues.size(); ++j) {
        const std::string& rightValue = rightColumnValues[j];
        bool matchFound = false;

        for (size_t i = 0; i < leftColumnValues.size(); ++i) {
            const std::string& leftValue = leftColumnValues[i];

            if (rightValue == leftValue) {
                for (const auto& columnPair : leftTableData) {
                    std::string colName = leftTable + "." + columnPair.first;
                    resultColumns[colName].push_back(columnPair.second[i]);
                }

                for (const auto& columnPair : rightTableData) {
                    std::string colName = rightTable + "." + columnPair.first;
                    resultColumns[colName].push_back(columnPair.second[j]);
                }

                matchFound = true;
                break;
            }
        }

        if (!matchFound) {
            for (const auto& columnPair : leftTableData) {
                std::string colName = leftTable + "." + columnPair.first;
                resultColumns[colName].push_back("NULL");
            }
            for (const auto& columnPair : rightTableData) {
                std::string colName = rightTable + "." + columnPair.first;
                resultColumns[colName].push_back(columnPair.second[j]);
            }
        }
    }

    resultMap[leftTable + "_" + rightTable] = resultColumns;
}

std::map<std::string, std::map<std::string, std::vector<std::string>>> SQLManager::getTableAndColumnFromDatabaseForJoins(
    const std::string& databaseName,
    const std::vector<Join>& joinData
) {
    json dbContent;
    if (!readDatabaseFile(databaseName, dbContent)) {
        std::cerr << "Error: Could not read database file." << std::endl;
        return {};
    }

    std::map<std::string, std::map<std::string, std::vector<std::string>>> resultTableColumns;

    std::set<std::string> requiredTables;
    std::set<std::string> requiredColumns;

    for (const auto& join : joinData) {
        requiredTables.insert(join.condition.leftTable);
        requiredTables.insert(join.condition.rightTable);
        requiredColumns.insert(join.condition.leftColumn);
        requiredColumns.insert(join.condition.rightColumn);
    }

    for (const auto& table : dbContent["tables"]) {
        std::string tableName = table["table_name"];

        if (requiredTables.find(tableName) == requiredTables.end()) {
            continue;
        }

        auto& columnMap = resultTableColumns[tableName];

        for (const auto& column : table["columns"]) {
            std::string columnName = column["name"];

            if (requiredColumns.find(columnName) == requiredColumns.end()) {
                continue;
            }

            columnMap[columnName] = {};

            for (const auto& row : table["rows"]) {
                if (row.contains(columnName)) {
                    columnMap[columnName].push_back(row[columnName].dump());
                } else {
                    columnMap[columnName].push_back("NULL");
                }
            }
        }
    }

    return resultTableColumns;
}

