#include "qbf.h"
#include <cmath>
#include <algorithm>
#include <iostream>

// 公開介面：呼叫遞迴起始點
QBFResult QBFSolver::solve(std::vector<Formula>& prefix, std::vector<std::vector<int>> matrix) {
    int number_of_clauses = matrix.size();

    int max_ID = 1;
    for(const auto& clause : matrix){
        for(int lit : clause){
            if(abs(lit)> max_ID){
                max_ID = abs(lit);
            }
        }
    }

    std::cout << "max_ID :" << max_ID <<std::endl;
    std::cout << "prefix size :" << (int)prefix.size() <<std::endl;

    // return Q_SAT;
    return solve_recursive(prefix, 0, matrix, ++max_ID);
}

// 核心 CEGAR 遞迴邏輯
QBFResult QBFSolver::solve_recursive(const std::vector<Formula>& prefix, int depth, std::vector<std::vector<int>> matrix, int next_ID) {
    // 1. 基底情況 (Base Cases)
    // 若矩陣為空，代表所有子句皆已滿足 -> SAT
    if (matrix.empty()) return Q_SAT;
    std::cout << "depth" << depth << std::endl;

    // 若矩陣中包含空子句 (代表出現了 False) -> UNSAT
    // for (const auto& c : matrix) {
    //     if (c.empty()) {
    //         std::cout << "empty clause" <<std::endl;
    //         return Q_UNSAT;
    //     }
    // }

    for (size_t i = 0; i < matrix.size(); i++) {
        if (matrix[i].empty()) {
            std::cout << "Empty clause found at index: " << i << std::endl;
            return Q_UNSAT;
        }
    }

    const Formula& currentQ = prefix[depth];
    // 2. 若已經處理完所有量詞，剩餘矩陣視為命題邏輯求解
    if (depth >= (int)prefix.size() - 1) {
        std::cout << "last layer" << std::endl;
        SATSolver sat;
        for (const auto& c : matrix) sat.addClause(c);
        if (currentQ.quantifier == 'a' && !sat.clauses.empty()) return Q_UNSAT;
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
    std::cout << "current Q :" << currentQ.vars[0] <<std::endl;
    SATSolver alpha;
    
    std::vector<int> vars_of_interest = currentQ.vars;
    
    int number_of_clauses = matrix.size();
    std::vector<int> var_b;
    for (int i = 0; i < number_of_clauses; i++){
        var_b.push_back((next_ID + i));
        vars_of_interest.push_back((next_ID + i));
    }
    if (currentQ.quantifier == 'e'){
        int i = 0;
        for (const auto& clause : matrix){
            std::vector<int> clause_p;
            for (int lit : clause){
                int var = std::abs(lit);
                for(int var_temp : currentQ.vars){
                    if (var == var_temp){
                        clause_p.push_back(lit);
                        break;
                    }
                }
            }
            clause_p.push_back(var_b[i]);
            alpha.addClause(clause_p);
            i += 1;
        }
    }
    if (currentQ.quantifier == 'a'){
        int i = 0;
        for (const auto& clause : matrix){
            for (int lit : clause){
                int var = std::abs(lit);
                for(int var_temp : currentQ.vars){
                    if (var == var_temp){
                        std::vector<int> clause_p;
                        clause_p.push_back(-lit);
                        clause_p.push_back(-var_b[i]);
                        alpha.addClause(clause_p);
                        break;
                    }
                }
            }
            i += 1;
        }
    }
    
    // 4. CEGAR 主迴圈
    while (true) {
        std::map<int, bool> b;
        std::map<int, bool> assignment;
        SATResult res = alpha.solve(assignment, vars_of_interest);

        std::cout << "Current Assignment (b variables):" << std::endl;
        for (const auto& [var, val] : assignment) {
            // 使用 C++17 的結構化綁定 (Structured Bindings)
            std::cout << "Variable " << var << " = " << (val ? "True" : "False") << std::endl;
        }
        std::cout << "--------------------------" << std::endl;

        // 如果抽象層無解
        if (res == S_UNSAT) {
            // ∃ 量詞找不到解 -> UNSAT; ∀ 量詞找不到反例 -> SAT
            return (currentQ.quantifier == 'e') ? Q_UNSAT : Q_SAT;
        }

        // 5. 處理下一詞傳遞的資訊
        std::vector<bool> next_top(number_of_clauses, false);
        for (int i = 0; i < number_of_clauses; i++){
            b.insert({(next_ID + i), assignment[i + next_ID]});
            next_top[i] = (assignment[i + next_ID])? false : true; //
        }

        // 5.1 簡化矩陣 (Substitution)
        auto simplified_matrix = simplify(matrix, currentQ, next_top);

        // 6. 遞迴求解內層
        QBFResult recursiveRes = solve_recursive(prefix, depth + 1, simplified_matrix, next_ID);

        // 7. 細化 (Refinement)
        if (currentQ.quantifier == 'e' && recursiveRes == Q_UNSAT) {
            // ∃ 賦值失敗 -> 加入封鎖子句
            std::cout << "e" << std::endl;
            alpha.addClause(generateRefinementClauseE(b, var_b));
        } 
        else if (currentQ.quantifier == 'a' && recursiveRes == Q_SAT) {
            // ∀ 嘗試的反例不成立 -> 加入封鎖子句
            std::cout << "a" << std::endl;
            alpha.addClause(generateRefinementClauseA(b, var_b));
        } 
        else {
            // 成功找到 Existential SAT 或 Universal UNSAT (反例)
            return (currentQ.quantifier == 'e') ? Q_SAT : Q_UNSAT;
        }
    }
}

// TODO: finish the removal of finished clauses.
// Remove the current variables 
std::vector<std::vector<int>> QBFSolver::simplify(const std::vector<std::vector<int>>& matrix, const Formula& currentQ, std::vector<bool> next_top) {
    std::vector<std::vector<int>> new_matrix;
    int i = 0;
    for (const auto& clause : matrix) {
        if(next_top[i] == false){
            std::vector<int> new_clause;

            for (int lit : clause) {
                int var = std::abs(lit);
                
                bool is_inQ = false;
                for (int var_Q : currentQ.vars){
                    if (var == var_Q) {
                        is_inQ = true;
                        break;
                    }
                }
                if (!is_inQ){
                    new_clause.push_back(lit);
                }
            }
                new_matrix.push_back(new_clause);            
        }
        i += 1;
    }
    return new_matrix;
}


// 生成封鎖子句 (Blocking Clause)
std::vector<int> QBFSolver::generateRefinementClauseE(std::map<int, bool>& b, const std::vector<int>& vars) {
    std::vector<int> clause;
    for (int v : vars) {
        if (b[v]) {
            // The v-th clause has to solve the problem.
            clause.push_back(-v);
        }
    }
    return clause;
}

// 生成封鎖子句 (Blocking Clause)
std::vector<int> QBFSolver::generateRefinementClauseA(std::map<int, bool>& b, const std::vector<int>& vars) {
    std::vector<int> clause;
    for (int v : vars) {
        if (!b[v]) {
            // The v-th clause has to avoid the problem.
            clause.push_back(v);
        }
    }
    return clause;
}
