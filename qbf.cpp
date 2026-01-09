

#include "qbf.h"
#include <cmath>
#include <algorithm>
#include <iostream>

// 公開介面：呼叫遞迴起始點
QBFResult QBFSolver::solve(std::vector<Formula>& prefix, std::vector<std::vector<int>> matrix) {
    int number_of_clauses = matrix.size();
    std::vector<bool> top(number_of_clauses, false);

    int max_ID = 1;
    for(std::vector<int> clause : matrix){
        for(int lit : clause){
            if(abs(lit)> max_ID){
                max_ID = abs(lit);
            }
        }
    }
    std::cout << ++max_ID << std::endl;

    // return Q_SAT;
    return solve_recursive(prefix, 0, matrix, top, ++max_ID);
}

// 核心 CEGAR 遞迴邏輯
QBFResult QBFSolver::solve_recursive(const std::vector<Formula>& prefix, int depth, std::vector<std::vector<int>> matrix, std::vector<bool> top, int next_ID) {
    // 1. 基底情況 (Base Cases)
    // 若矩陣為空，代表所有子句皆已滿足 -> SAT
    if (matrix.empty()) return Q_SAT;
    
    // 若矩陣中包含空子句 (代表出現了 False) -> UNSAT
    for (const auto& c : matrix) {
        if (c.empty()) return Q_UNSAT;
    }

    // 2. 若已經處理完所有量詞，剩餘矩陣視為命題邏輯求解
    if (depth >= (int)prefix.size()) {
        SATSolver sat;
        for (const auto& c : matrix) sat.addClause(c);
        std::map<int, bool> dummy_assignment;
        // 收集剩餘矩陣中的所有變數
        std::vector<int> remaining_vars;
        for (const auto& c : matrix) {
            for (int lit : c) {
                int v = std::abs(lit);
                remaining_vars.push_back(v);
            }
        }
        // 移除重複變數
        std::sort(remaining_vars.begin(), remaining_vars.end());
        remaining_vars.erase(std::unique(remaining_vars.begin(), remaining_vars.end()), remaining_vars.end());

        return (sat.solve(dummy_assignment, remaining_vars) == S_SAT) ? Q_SAT : Q_UNSAT;
    }

    // 3. 準備當前層級的抽象 (Abstraction)
    const Formula& currentQ = prefix[depth];
    SATSolver alpha;

    // 投影 (Projection)：將只包含當前層級變數的子句加入 alpha，增加剪枝效率
    for (const auto& clause : matrix) {
        bool all_in_current_scope = true;
        for (int lit : clause) {
            int var = std::abs(lit);
            bool is_current_var = false;
            for (int v : currentQ.vars) {
                if (var == v) { is_current_var = true; break; }
            }
            if (!is_current_var) { all_in_current_scope = false; break; }
        }
        if (all_in_current_scope) alpha.addClause(clause);
    }


    // 4. CEGAR 主迴圈
    while (true) {
        std::map<int, bool> b;
        SATResult res = alpha.solve(b, currentQ.vars);

        // 如果抽象層無解
        if (res == S_UNSAT) {
            // ∃ 量詞找不到解 -> UNSAT; ∀ 量詞找不到反例 -> SAT
            // return (currentQ.quantifier == 'e') ? Q_UNSAT : Q_SAT;
            return Q_UNSAT;
        }

        // 5. 簡化矩陣 (Substitution)
        auto simplified_matrix = simplify(matrix, b);

        // 6. 遞迴求解內層
        QBFResult recursiveRes = solve_recursive(prefix, depth + 1, simplified_matrix, top, next_ID);

        // 7. 細化 (Refinement)
        if (currentQ.quantifier == 'e' && recursiveRes == Q_UNSAT) {
            // ∃ 賦值失敗 -> 加入封鎖子句
            alpha.addClause(generateRefinementClause(b, currentQ.vars));
        } 
        else if (currentQ.quantifier == 'a' && recursiveRes == Q_SAT) {
            // ∀ 嘗試的反例不成立 -> 加入封鎖子句
            alpha.addClause(generateRefinementClause(b, currentQ.vars));
        } 
        else {
            // 成功找到 Existential SAT 或 Universal UNSAT (反例)
            return (currentQ.quantifier == 'e') ? Q_SAT : Q_UNSAT;
        }
    }
}

// 根據賦值簡化公式
std::vector<std::vector<int>> QBFSolver::simplify(const std::vector<std::vector<int>>& matrix, std::map<int, bool>& b) {
    std::vector<std::vector<int>> new_matrix;
    for (const auto& clause : matrix) {
        bool satisfied = false;
        std::vector<int> new_clause;

        for (int lit : clause) {
            int var = std::abs(lit);
            bool is_pos = (lit > 0);

            if (b.count(var)) {
                if (b[var] == is_pos) {
                    satisfied = true; // 子句已滿足
                    break;
                }
                // 如果文字為假，則跳過 (不加入 new_clause)，達成削減效果
            } else {
                new_clause.push_back(lit); // 內層變數，保留
            }
        }

        if (!satisfied) {
            new_matrix.push_back(new_clause);
        }
    }
    return new_matrix;
}

// 生成封鎖子句 (Blocking Clause)
std::vector<int> QBFSolver::generateRefinementClause(std::map<int, bool>& b, const std::vector<int>& vars) {
    std::vector<int> clause;
    for (int v : vars) {
        if (b.count(v)) {
            // 封鎖目前賦值：原本是 True 就放 -v，原本是 False 就放 v
            clause.push_back(b[v] ? -v : v);
        }
    }
    return clause;
}

// #include "qbf.h"
// #include <cmath>
// #include <algorithm>
// #include <iostream>

// // 公開介面：呼叫遞迴起始點
// QBFResult QBFSolver::solve(std::vector<Formula>& prefix, std::vector<std::vector<int>> matrix) {
//     return solve_recursive(prefix, 0, matrix);
// }

// // 核心 CEGAR 遞迴邏輯
// QBFResult QBFSolver::solve_recursive(const std::vector<Formula>& prefix, int depth, std::vector<std::vector<int>> matrix) {
//     // 1. 基底情況 (Base Cases)
//     // 若矩陣為空，代表所有子句皆已滿足 -> SAT
//     if (matrix.empty()) return Q_SAT;
    
//     // 若矩陣中包含空子句 (代表出現了 False) -> UNSAT
//     for (const auto& c : matrix) {
//         if (c.empty()) return Q_UNSAT;
//     }

//     // 2. 若已經處理完所有量詞，剩餘矩陣視為命題邏輯求解
//     if (depth >= (int)prefix.size()) {
//         SATSolver sat;
//         for (const auto& c : matrix) sat.addClause(c);
//         std::map<int, bool> dummy_assignment;
//         // 收集剩餘矩陣中的所有變數
//         std::vector<int> remaining_vars;
//         for (const auto& c : matrix) {
//             for (int lit : c) {
//                 int v = std::abs(lit);
//                 remaining_vars.push_back(v);
//             }
//         }
//         // 移除重複變數
//         std::sort(remaining_vars.begin(), remaining_vars.end());
//         remaining_vars.erase(std::unique(remaining_vars.begin(), remaining_vars.end()), remaining_vars.end());

//         return (sat.solve(dummy_assignment, remaining_vars) == S_SAT) ? Q_SAT : Q_UNSAT;
//     }

//     // 3. 準備當前層級的抽象 (Abstraction)
//     const Formula& currentQ = prefix[depth];
//     SATSolver alpha;

//     // 投影 (Projection)：將只包含當前層級變數的子句加入 alpha，增加剪枝效率
//     for (const auto& clause : matrix) {
//         bool all_in_current_scope = true;
//         for (int lit : clause) {
//             int var = std::abs(lit);
//             bool is_current_var = false;
//             for (int v : currentQ.vars) {
//                 if (var == v) { is_current_var = true; break; }
//             }
//             if (!is_current_var) { all_in_current_scope = false; break; }
//         }
//         if (all_in_current_scope) alpha.addClause(clause);
//     }

//     // 4. CEGAR 主迴圈
//     while (true) {
//         std::map<int, bool> b;
//         SATResult res = alpha.solve(b, currentQ.vars);

//         // 如果抽象層無解
//         if (res == S_UNSAT) {
//             // ∃ 量詞找不到解 -> UNSAT; ∀ 量詞找不到反例 -> SAT
//             // return (currentQ.quantifier == 'e') ? Q_UNSAT : Q_SAT;
//             return Q_UNSAT;
//         }

//         // 5. 簡化矩陣 (Substitution)
//         auto simplified_matrix = simplify(matrix, b);

//         // 6. 遞迴求解內層
//         QBFResult recursiveRes = solve_recursive(prefix, depth + 1, simplified_matrix);

//         // 7. 細化 (Refinement)
//         if (currentQ.quantifier == 'e' && recursiveRes == Q_UNSAT) {
//             // ∃ 賦值失敗 -> 加入封鎖子句
//             alpha.addClause(generateRefinementClause(b, currentQ.vars));
//         } 
//         else if (currentQ.quantifier == 'a' && recursiveRes == Q_SAT) {
//             // ∀ 嘗試的反例不成立 -> 加入封鎖子句
//             alpha.addClause(generateRefinementClause(b, currentQ.vars));
//         } 
//         else {
//             // 成功找到 Existential SAT 或 Universal UNSAT (反例)
//             return (currentQ.quantifier == 'e') ? Q_SAT : Q_UNSAT;
//         }
//     }
// }

// // 根據賦值簡化公式
// std::vector<std::vector<int>> QBFSolver::simplify(const std::vector<std::vector<int>>& matrix, std::map<int, bool>& b) {
//     std::vector<std::vector<int>> new_matrix;
//     for (const auto& clause : matrix) {
//         bool satisfied = false;
//         std::vector<int> new_clause;

//         for (int lit : clause) {
//             int var = std::abs(lit);
//             bool is_pos = (lit > 0);

//             if (b.count(var)) {
//                 if (b[var] == is_pos) {
//                     satisfied = true; // 子句已滿足
//                     break;
//                 }
//                 // 如果文字為假，則跳過 (不加入 new_clause)，達成削減效果
//             } else {
//                 new_clause.push_back(lit); // 內層變數，保留
//             }
//         }

//         if (!satisfied) {
//             new_matrix.push_back(new_clause);
//         }
//     }
//     return new_matrix;
// }

// // 生成封鎖子句 (Blocking Clause)
// std::vector<int> QBFSolver::generateRefinementClause(std::map<int, bool>& b, const std::vector<int>& vars) {
//     std::vector<int> clause;
//     for (int v : vars) {
//         if (b.count(v)) {
//             // 封鎖目前賦值：原本是 True 就放 -v，原本是 False 就放 v
//             clause.push_back(b[v] ? -v : v);
//         }
//     }
//     return clause;
// }