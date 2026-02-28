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
    QBFResult solve_recursive(const std::vector<Formula>& prefix, int depth, std::vector<std::vector<int>> matrix, int next_var);
    std::vector<std::vector<int>> simplify(const std::vector<std::vector<int>>& matrix, const Formula& currentQ, std::vector<bool> next_top);
    std::vector<int> generateRefinementClauseE(std::map<int, bool>& b, const std::vector<int>& vars);
    std::vector<int> generateRefinementClauseA(std::map<int, bool>& b, const std::vector<int>& vars);
};

#endif