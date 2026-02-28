#ifndef SAT_H
#define SAT_H

#include <vector>
#include <map>
#include <cryptominisat.h>

enum SATResult { S_SAT, S_UNSAT, S_UNKNOWN };

class SATSolver {
public:
    SATSolver();
    ~SATSolver();

    // 依照你原本的呼叫方式：addClause(std::vector<int>)
    void addClause(const std::vector<int>& clause);

    // 依照你原本的呼叫方式：solve(map, vector<int>)
    SATResult solve(std::map<int, bool>& assignment, const std::vector<int>& vars_of_interest);

    // 為了相容你 code 中的 sat.clauses.empty() 判斷
    std::vector<std::vector<int>> clauses; 

private:
    CMSat::SATSolver solver;
    void ensure_vars(int max_var_id);
};

#endif