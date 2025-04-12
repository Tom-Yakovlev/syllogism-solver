#include "ProofSolver.h"
#include "Utils.h"
#include <cassert>
#include <sstream>
#include <iostream>

// ANSI color codes
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define RESET   "\033[0m"

void runTest(const std::string& premisesStr, const std::string& conclusion, const std::string& expectedLastLine) {
    std::cout << "[" << expectedLastLine.substr(expectedLastLine.find(':') + 1) << "] Testing: " << conclusion << "\n";
    ProofSolver solver;
    solver.enableBeautify(false);
    solver.setInput(premisesStr, conclusion);

    solver.solve();

    std::stringstream out;
    std::streambuf* oldCout = std::cout.rdbuf(out.rdbuf());
    solver.displayProof();
    std::cout.rdbuf(oldCout);

    std::string proofOutput = out.str();
    if (proofOutput.find(expectedLastLine) == std::string::npos) {
        std::cerr << RED << "Test failed for conclusion: " << conclusion << RESET << "\n";
        std::cerr << RED << "Expected to find line: " << expectedLastLine << RESET << "\n";
        std::cerr << RED << "Actual proof output:\n" << proofOutput << RESET << "\n";
        assert(false);
    } else {
        std::cout << GREEN << "Passed: " << conclusion << RESET << "\n";
    }
}

int main() {
    std::cout << "=== Basic Rules ===\n";
    std::cout << "[MP] "; runTest("P,P->Q", "Q", "Q    :MP 2 3");
    std::cout << "[MT] "; runTest("~Q,P->Q", "~P", "~P    :MT 2 3");
    std::cout << "[MTP] "; runTest("PvQ,~P", "Q", "Q    :MTP 2 3");
    std::cout << "[DNE] "; runTest("~~P", "P", "P    :DNE 2");
    std::cout << "[DNI] "; runTest("P", "~~P", "~~P    :DNI 2");
    std::cout << "[S] "; runTest("P^Q", "P", "P    :S 2");
    std::cout << "[ADJ] "; runTest("P,Q", "P^Q", "P^Q    :ADJ 2 3");
    std::cout << "[ADD] "; runTest("P", "Pvψ", "Pvψ    :ADD 2");
    std::cout << "[BC] "; runTest("P<->Q,Q->P", "P->Q", "P->Q    :BC 2 3");
    std::cout << "[CB] "; runTest("P->Q,Q->P", "P<->Q", "P<->Q    :CB 2 3");

    std::cout << "\n=== Derived Rules ===\n";
    std::cout << "[D-HS] "; runTest("P->Q,Q->R", "P->R", "P->R    :D-HS 2 3");
    std::cout << "[D-MCC] "; runTest("Q", "X->Q", "X->Q    :D-MCC 2");
    std::cout << "[D-MCNA] "; runTest("~P", "P->X", "P->X    :D-MCNA 2");
    std::cout << "[D-CPO] "; runTest("P->Q", "~Q->~P", "~Q->~P    :D-CPO 2");
    std::cout << "[D-CPT] "; runTest("~P->~Q", "Q->P", "Q->P    :D-CPT 2");
    std::cout << "[D-DIL] "; runTest("~P->Q,P->Q", "Q", "Q    :D-DIL 2 3");
    std::cout << "[D-CM] "; runTest("~P->P", "P", "P    :D-CM 2");
    std::cout << "[D-EFQ] "; runTest("P,~P", "R", "R    :D-EFQ 2 3");

    std::cout << "\n=== De Morgan Laws ===\n";
    std::cout << "[D-SDMO] "; runTest("(P^Q)<->~(~Pv~Q)", "(P^Q)<->~(~Pv~Q)", "(P^Q)<->~(~Pv~Q)    :D-SDMO 2");
    std::cout << "[D-DMO] "; runTest("~(PvQ)<->(~P^~Q)", "~(PvQ)<->(~P^~Q)", "~(PvQ)<->(~P^~Q)    :D-DMO 2");
    std::cout << "[D-DMT] "; runTest("~(P^Q)<->(~Pv~Q)", "~(P^Q)<->(~Pv~Q)", "~(P^Q)<->(~Pv~Q)    :D-DMT 2");
    std::cout << "[D-SDMT] "; runTest("(PvQ)<->~(~P^~Q)", "(PvQ)<->~(~P^~Q)", "(PvQ)<->~(~P^~Q)    :D-SDMT 2");
    std::cout << "[D-NC] "; runTest("~(P->Q)<->(P^~Q)", "~(P->Q)<->(P^~Q)", "~(P->Q)<->(P^~Q)    :D-NC 2");

    std::cout << "\n=== Composite Proof ===\n";
    std::cout << "[D-PBC] "; runTest("P->R,PvQ,Q->R", "R", "R    :D-PBC 2 3 4");

    std::cout << "\n=== Reverse Direction & Inference Tests ===\n";
    std::cout << "[BC Reverse] "; runTest("P<->Q,Q->P", "P->Q", "P->Q    :BC 2 3");
    std::cout << "[CB Reverse] "; runTest("Q->P,P->Q", "P<->Q", "P<->Q    :CB 2 3");
    std::cout << "[D-CPO Reverse] "; runTest("~Q->~P", "P->Q", "P->Q    :D-CPO 2");
    std::cout << "[D-CPT Reverse] "; runTest("Q->P", "~P->~Q", "~P->~Q    :D-CPT 2");
    std::cout << "[D-DIL Reverse] "; runTest("P->Q,~P->Q", "Q", "Q    :D-DIL 2 3");
    std::cout << "[D-NC Reverse] "; runTest("(P^~Q)<->~(P->Q)", "~(P->Q)<->(P^~Q)", "~(P->Q)<->(P^~Q)    :D-NC 2");



    std::cout << "\nAll tests passed.\n";
    return 0;
}
