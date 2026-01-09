#ifndef SATSOLVER_H
#define SATSOLVER_H

#include <vector>
#include <map>
#include <set>

enum SATResult { S_SAT, S_UNSAT };

class SATSolver {
public:
    // 加入子句到當前的 SAT 問題中
    void addClause(const std::vector<int>& clause);

    // 求解並回傳 assignment，vars_of_interest 指定需要賦值的變數
    SATResult solve(std::map<int, bool>& assignment, const std::vector<int>& vars_of_interest);

    // 清空目前的子句集，準備下一次求解
    void reset();

private:
    std::vector<std::vector<int>> clauses;
    
    // 內部遞迴求解函數
    bool backtrack(std::map<int, bool>& current_assignment, std::vector<int> target_vars, int index);
    
    // 檢查當前賦值是否違反任何子句
    bool is_consistent(const std::map<int, bool>& assignment);
};

#endif