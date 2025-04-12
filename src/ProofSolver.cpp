#include "ProofSolver.h"
#include "Utils.h"
#include "Rules.h"
#include <iostream>
#include <sstream>
#include <unordered_set>

void ProofSolver::readInput() {
    std::string input;
    std::cout << "\nEnter premises separated by commas (end with ';'):\n";
    std::getline(std::cin, input, ';');

    std::stringstream ss(input);
    std::string item;
    while (std::getline(ss, item, ',')) {
        premises.push_back(trim(item));
    }

    std::cout << "Enter the conclusion (end with ';'):\n";
    std::getline(std::cin, conclusion, ';');
    conclusion = trim(conclusion);
}

void ProofSolver::addRule(const Rule& rule) {
    rules.push_back(rule);
}

void ProofSolver::solve() {
    // Load predefined rules
    rules = getAllRules();

    // Add Show line as first
    proofLines.push_back({
        1,
        "Show: " + trim(conclusion),
        "",
        {}
    });

    // Step 1: Seed premises into proofLines
    int lineNum = 2;
    for (const auto& p : premises) {
        proofLines.push_back({lineNum++, trim(p), "PR", {}});
    }

    // Step 2: Iteratively apply rules
    bool progress = true;
    std::unordered_set<std::string> seen;
    for (const auto& stmt : proofLines) {
        seen.insert(trim(stmt.expression));
    }

    while (progress) {
        progress = false;

        for (const auto& rule : rules) {
            size_t n = rule.numPremises;

            std::vector<int> indices(proofLines.size());
            for (size_t i = 0; i < proofLines.size(); ++i)
                indices[i] = static_cast<int>(i);

            std::vector<std::vector<int>> combos;

            // Generate all n-length combinations
            std::function<void(size_t, std::vector<int>)> generateCombos =
                [&](size_t start, std::vector<int> current) {
                    if (current.size() == n) {
                        combos.push_back(current);
                        return;
                    }
                    for (size_t i = start; i < indices.size(); ++i) {
                        auto temp = current;
                        temp.push_back(indices[i]);
                        generateCombos(i + 1, temp);
                    }
                };
            generateCombos(0, {});

            for (const auto& combo : combos) {
                std::vector<std::string> exprs;
                for (int idx : combo) {
                    exprs.push_back(trim(proofLines[idx].expression));
                }

                std::optional<std::string> result = rule.apply(exprs);
                if (result) {
                    std::string trimmedResult = trim(*result);
                    if (seen.find(trimmedResult) == seen.end()) {
                        std::vector<int> refs;
                        for (int idx : combo)
                            refs.push_back(proofLines[idx].lineNumber);

                        proofLines.push_back({
                            static_cast<int>(proofLines.size()) + 1,
                            trimmedResult,
                            rule.name,
                            refs
                        });

                        seen.insert(trimmedResult);
                        progress = true;

                        if (trimmedResult == trim(conclusion)) return;
                    }
                }
            }
        }
    }
}

void ProofSolver::displayProof() const {
    std::cout << "=== Proof Steps ===\n";
    for (const auto& stmt : proofLines) {
        if (stmt.lineNumber == 1) {
            std::cout << stmt.lineNumber << ". " << stmt.expression << "\n";
        } else {
            std::cout << stmt.lineNumber << ".  " << stmt.expression
                      << "    :" << stmt.justification;

            if (!stmt.references.empty()) {
                std::cout << " ";
                for (size_t i = 0; i < stmt.references.size(); ++i) {
                    std::cout << stmt.references[i];
                    if (i < stmt.references.size() - 1) std::cout << " ";
                }
            }

            std::cout << "\n";
        }
    }
}
