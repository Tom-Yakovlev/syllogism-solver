#include <iostream>
#include "ProofSolver.h"

int main() {
    while (true) {
        ProofSolver solver;
        solver.readInput();
        solver.solve();
        solver.displayProof();

        std::cout << "\nEnter another proof, or press Ctrl+C to quit.\n\n";
    }
}
