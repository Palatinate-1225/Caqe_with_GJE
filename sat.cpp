#include "sat.h"
#include <cmath>
#include <algorithm>

SATSolver::SATSolver() {
    // 可以在這裡設定 CMS 參數，例如執行緒數量
    solver.set_num_threads(1); 
}

SATSolver::~SATSolver() {}

void SATSolver::ensure_vars(int max_var_id) {
    // CMS 需要手動增加變數數量
    while (max_var_id >= (int)solver.nVars()) {
        solver.new_var();
    }
}

void SATSolver::addClause(const std::vector<int>& clause) {
    // 同步更新你原本代碼中檢查用的 clauses vector
    clauses.push_back(clause);

    std::vector<CMSat::Lit> cms_lits;
    for (int lit : clause) {
        int var = std::abs(lit) - 1; // 轉為 0-indexed
        ensure_vars(var);
        // CMSat::Lit(變數編號, 是否為負)
        cms_lits.push_back(CMSat::Lit(var, lit < 0));
    }
    solver.add_clause(cms_lits);
}

SATResult SATSolver::solve(std::map<int, bool>& assignment, const std::vector<int>& vars_of_interest) {
    CMSat::lbool res = solver.solve();

    if (res == CMSat::l_True) {
        assignment.clear();
        const std::vector<CMSat::lbool> model = solver.get_model();
        
        for (int v : vars_of_interest) {
            int var_idx = std::abs(v) - 1;
            if (var_idx < (int)model.size()) {
                // 如果 CMS 返回 l_True 代表真，l_False 代表假
                assignment[v] = (model[var_idx] == CMSat::l_True);
            } else {
                assignment[v] = false; // 沒出現過的變數預設給 false
            }
        }
        return S_SAT;
    } else if (res == CMSat::l_False) {
        return S_UNSAT;
    }
    return S_UNKNOWN;
}