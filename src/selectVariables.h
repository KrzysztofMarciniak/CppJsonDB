#pragma once

typedef std::string JoinType;

struct JoinCondition {
    std::string leftTable;
    std::string leftColumn;
    std::string rightTable;
    std::string rightColumn;
};
struct Join {
    std::string table;
    std::string joinType;
    JoinCondition condition;

    Join() = default;
    Join(const std::string& t, const std::string& jt, const JoinCondition& cond)
        : table(t), joinType(jt), condition(cond) {}
};
