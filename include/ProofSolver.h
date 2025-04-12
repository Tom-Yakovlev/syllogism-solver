#ifndef PROOFSOLVER_H
#define PROOFSOLVER_H

#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <unordered_set>
#include <unordered_map>

// Represents a single proof line
struct Statement {
    int lineNumber;
    std::string expression;
    std::string justification;
    std::vector<int> references;
    int indentLevel = 0;  // 0 = top-level, increases for subproofs
};

// Represents a logical inference rule
struct Rule {
    std::string name;
    int numPremises;
    std::function<std::optional<std::string>(const std::vector<std::string>&)> apply;
};

class ProofSolver {

public:

    void readInput();
    void addRule(const Rule& rule);
    void solve(); // Solver logic (forward chaining)
    void solveBackward(); // Backward chaining entry point
    bool prove(const std::string& goal); // Recursive backward chaining prover
    int getLineNumberFor(const std::string& expr) const;

    void displayProof() const;
    void enableBeautify(bool enable);
    void setInput(const std::string& premisesStr, const std::string& conclusionStr);

private:

    void startSubproof(const std::string& formula); // inserts Show: and AS
    void endSubproof(const std::string& rule, const std::vector<int>& refs); // inserts QED

    bool tryConditionalDerivation(const std::string& implicationStr);
    bool tryDirectDerivation(const std::string& goal);

    bool wasConclusionDerived() const;
    const std::vector<Statement>& getProofLines() const;

    // Backward chaining helpers
    std::unordered_set<std::string> proven;
    std::unordered_map<std::string, int> lineMap;
    std::unordered_set<std::string> visitedGoals;
    std::vector<std::vector<std::string>> guessPremisesFor(const Rule& rule, const std::string& goal);
    int addProofLine(const std::string& expr, const std::string& justification, const std::vector<int>& refs);

    bool beautify = false; // connective beautifier flag

    std::vector<Statement> proofLines;
    std::vector<std::string> premises;
    std::string conclusion;
    std::vector<Rule> rules;

    std::vector<int> showStack; // tracks active subproofs (stores indent levels)
    int currentIndent = 0;      // current depth level

};

#endif // PROOFSOLVER_H