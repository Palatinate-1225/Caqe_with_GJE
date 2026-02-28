#include "sat.h"
#include <cmath>
#include <algorithm>

void SATSolver::addClause(const std::vector<int>& clause) {
    clauses.push_back(clause);
}

void SATSolver::reset() {
    clauses.clear();
}

SATResult SATSolver::solve(std::map<int, bool>& assignment, const std::vector<int>& vars_of_interest) {
    assignment.clear();
    // 開始遞迴搜尋
    if (backtrack(assignment, vars_of_interest, 0)) {
        return S_SAT;
    }
    return S_UNSAT;
}

bool SATSolver::backtrack(std::map<int, bool>& current_assignment, std::vector<int> target_vars, int index) {
    // 基底情況：所有感興趣的變數都已賦值
    if (index == target_vars.size()) {
        return is_consistent(current_assignment);
    }

    int var = target_vars[index];

    // 嘗試 False
    current_assignment[var] = false;
    if (is_consistent(current_assignment)) {
        if (backtrack(current_assignment, target_vars, index + 1)) return true;
    }

    // 嘗試 True
    current_assignment[var] = true;
    if (is_consistent(current_assignment)) {
        if (backtrack(current_assignment, target_vars, index + 1)) return true;
    }

    // 找不到解，回溯
    current_assignment.erase(var);
    return false;
}

bool SATSolver::is_consistent(const std::map<int, bool>& assignment) {
    if (clauses.empty()) return true;
    for (const auto& clause : clauses) {
        bool clause_satisfied = false;
        bool has_unassigned = false;

        for (int lit : clause) {
            int var = std::abs(lit);
            bool is_pos = (lit > 0);

            if (assignment.count(var)) {
                if (assignment.at(var) == is_pos) {
                    clause_satisfied = true;
                    break;
                }
            } else {
                has_unassigned = true;
                break; // to accelerate
            }
        }

        // 如果子句中所有文字都已賦值且都不滿足，則此賦值無效
        if (!clause_satisfied && !has_unassigned) {
            return false;
        }
    }
    return true;
}