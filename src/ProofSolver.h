#ifndef PROOFSOLVER_H
#define PROOFSOLVER_H

#include <string>
#include <vector>
#include <functional>
#include <optional>

// Represents a single proof line
struct Statement {
    int lineNumber;
    std::string expression;
    std::string justification;
    std::vector<int> references;
};

// Represents a logical inference rule
struct Rule {
    std::string name;
    int numPremises;
    std::function<std::optional<std::string>(const std::vector<std::string>&)> apply;
};

class ProofSolver {
private:
    std::vector<Statement> proofLines;
    std::vector<std::string> premises;
    std::string conclusion;
    std::vector<Rule> rules;
    
    bool wasConclusionDerived() const;
    const std::vector<Statement>& getProofLines() const;

public:
    void readInput();
    void addRule(const Rule& rule);
    void solve(); // Solver logic (to be implemented)
    void displayProof() const;
};

#endif // PROOFSOLVER_H
