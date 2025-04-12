#include "ProofSolver.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    bool useBeautify = false;

    if (argc > 1 && std::string(argv[1]) == "--pretty") {
        useBeautify = true;
    }

    while (true) {
        ProofSolver solver;
        solver.enableBeautify(useBeautify);
        solver.readInput();
        solver.solve();
        solver.displayProof();

        std::cout << "\nEnter another proof, or press Ctrl+C to quit.\n\n";
    }
}
