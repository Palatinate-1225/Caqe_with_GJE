#ifndef QBFSOLVER_H
#define QBFSOLVER_H

#include "sat.h"
#include <vector>

enum QBFResult { Q_SAT, Q_UNSAT };

class QBFSolver {
public:
    struct Formula {
        char quantifier; // 'e' or 'a'
        std::vector<int> vars;
    };

    QBFResult solve(std::vector<Formula>& prefix, std::vector<std::vector<int>> matrix);

private:
    QBFResult solve_recursive(const std::vector<Formula>& prefix, int depth, std::vector<std::vector<int>> matrix, std::vector<bool> top, int next_var);
    std::vector<std::vector<int>> simplify(const std::vector<std::vector<int>>& matrix, std::map<int, bool>& b);
    std::vector<int> generateRefinementClause(std::map<int, bool>& b, const std::vector<int>& vars);
};

#endif