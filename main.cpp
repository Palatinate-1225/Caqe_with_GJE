
#include "qbf.h"
#include <iostream>

int main() {
    QBFSolver solver;

    // 範例：forall x1, exists x2 . (x1 != x2)
    std::vector<QBFSolver::Formula> prefix = {
        {'a', {1}}, 
        {'e', {2, 3}}
    };
    std::vector<std::vector<int>> matrix = { {1, 2}};


    QBFResult res = solver.solve(prefix, matrix);
    std::cout << "QBF Result: " << (res == Q_SAT ? "SAT" : "UNSAT") << std::endl;

    return 0;
}

// SAT solver test

// #include "sat.h"
// #include <iostream>

// int main(){
//     SATSolver solver;

//     std::vector<int> clause1 = {1, 3}; 
//     solver.addClause(clause1);
//     std::vector<int> clause2 = {2, 4}; 
//     solver.addClause(clause2);
    
//     std::vector<int> my_var = {1, 2, 3, 4};
//     std::map<int, bool> result_map;
//     SATResult res = solver.solve(result_map, my_var);

//     if (res == S_SAT) {
//         std::cout << "SAT" << std::endl;
//         for (auto const& [var, val] : result_map) {
//             std::cout << "變數 " << var << " = " << (val ? "True" : "False") << std::endl;
//         }
//     } else {
//         std::cout << "UNSAT" << std::endl;
//     }

//     return 0;
// }