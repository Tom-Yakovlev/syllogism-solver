#include "ProofSolver.h"
#include "Utils.h"
#include "Rules.h"
#include <iostream>
#include <sstream>
#include <unordered_set>

void ProofSolver::readInput() {
    std::string input;

    std::cout << "\nEnter premises separated by commas:\n";
    std::getline(std::cin, input);

    std::stringstream ss(input);
    std::string item;
    premises.clear();  // Clear any leftover premises
    while (std::getline(ss, item, ',')) {
        premises.push_back(normalizeConnectives(trim(item)));
    }

    std::cout << "Enter the conclusion:\n";
    std::getline(std::cin, conclusion);
    conclusion = normalizeConnectives(trim(conclusion));
}

void ProofSolver::addRule(const Rule& rule) {
    rules.push_back(rule);
}

void ProofSolver::solve() {
    rules = getAllRules();
    proofLines.push_back({1, "Show: " + trim(conclusion), "", {}, 0});
    showStack.push_back(0);
    currentIndent = 0;

    int lineNum = 2;
    for (const auto& p : premises) {
        proofLines.push_back({lineNum++, trim(p), "PR", {}, currentIndent});
    }

    std::unordered_set<std::string> attempted;
    if (tryConditionalDerivation(conclusion, attempted)) return;

    if (tryDirectDerivation(conclusion)) {
        displayProof();
        return;
    }

    for (const auto& stmt : proofLines) {
        if (normalizeConnectives(trim(stmt.expression)) == normalizeConnectives(trim(conclusion))) {
            displayProof();
            return;
        }
    }

    std::cout << "[Running fallback rule application...]\n";

    bool progress = true;
    std::unordered_set<std::string> seen;
    for (const auto& stmt : proofLines) {
        seen.insert(normalizeConnectives(trim(stmt.expression)));
    }

    int iterationCount = 0;
    int derivationCount = 0;

    while (progress) {
    progress = false;
    iterationCount++;
    if (iterationCount > 1000) {
        std::cerr << "[ERROR] Aborting solve() — rule application exceeded 1000 iterations.\n";
        break;
    }

    for (const auto& rule : rules) {
        size_t n = rule.numPremises;
        std::vector<int> indices(proofLines.size());
        for (size_t i = 0; i < proofLines.size(); ++i)
            indices[i] = static_cast<int>(i);

        std::vector<std::vector<int>> combos;
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
            bool skipCombo = false;

            for (int idx : combo) {
                std::string expr = trim(proofLines[idx].expression);
                if (expr.rfind("Show:", 0) == 0) {
                    skipCombo = true;
                    break;
                }
                exprs.push_back(normalizeConnectives(expr));
            }

            if (skipCombo || exprs.size() != rule.numPremises)
                continue;

            std::optional<std::string> result = rule.apply(exprs);
            if (result) {
                std::string trimmedResult = normalizeConnectives(trim(*result));
                if (seen.find(trimmedResult) == seen.end()) {
                    derivationCount++;
                    if (derivationCount % 100 == 0) {
                        std::cout << "[INFO] Derived " << derivationCount << " formulas...\n";
                    }

                    std::cout << "[DEBUG] Derived: " << trimmedResult << "    :" << rule.name << "\n";

                    std::vector<int> refs;
                    for (int idx : combo)
                        refs.push_back(proofLines[idx].lineNumber);

                    proofLines.push_back({
                        static_cast<int>(proofLines.size()) + 1,
                        trimmedResult,
                        rule.name,
                        refs,
                        currentIndent
                    });

                    seen.insert(trimmedResult);
                    progress = true;

                    if (trimmedResult == normalizeConnectives(trim(conclusion))) return;
                }
            }
        }
    }
}
}

// Helper Function for solver()
bool ProofSolver::tryConditionalDerivation(const std::string& implicationStr, std::unordered_set<std::string>& attempted) {

    static int cdDepth = 0;
    cdDepth++;

    if (cdDepth > 10) {
        std::cerr << "[ERROR] Maximum CD recursion depth exceeded.\n";
        cdDepth--;
        return false;
    }

    std::string normalized = normalizeConnectives(trim(implicationStr));

    if (attempted.count(normalized)) {
        std::cerr << "[CD] Skipping already-attempted implication: " << normalized << "\n";
        return false;
    }
    attempted.insert(normalized);

    auto implication = splitImplication(implicationStr);
    if (!implication) {
        cdDepth--;
        return false;
    }

    std::string antecedent = implication->first;
    std::string consequent = implication->second;

    startSubproof(antecedent); // Show: antecedent + AS

    std::unordered_set<std::string> seen;
    for (const auto& stmt : proofLines) {
        seen.insert(normalizeConnectives(trim(stmt.expression)));
    }

    // Try to close the subproof directly first
    if (tryDirectDerivation(consequent)) {
        int qedLine = static_cast<int>(proofLines.size());
        endSubproof("CD", {qedLine});
        cdDepth--;
        return true;
    }

    // Try rules + recursive CD
    bool progress = true;
    int stallCounter = 0;

    while (progress) {
        progress = false;
        stallCounter++;
        if (stallCounter > 100) {
            std::cerr << "[ERROR] CD subproof stalled — no progress after 100 cycles.\n";
            cdDepth--;
            return false;
        }

        for (const auto& rule : rules) {
            size_t n = rule.numPremises;
            std::vector<int> indices(proofLines.size());
            for (size_t i = 0; i < indices.size(); ++i) indices[i] = static_cast<int>(i);

            std::vector<std::vector<int>> combos;
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
                bool skipCombo = false;

                for (int idx : combo) {
                    std::string expr = trim(proofLines[idx].expression);
                    if (expr.rfind("Show:", 0) == 0) {
                        skipCombo = true;
                        break;
                    }
                    exprs.push_back(normalizeConnectives(expr));
                }

                if (skipCombo || exprs.size() != rule.numPremises) continue;

                std::optional<std::string> result = rule.apply(exprs);
                if (!result) continue;

                std::string trimmedResult = normalizeConnectives(trim(*result));
                if (seen.count(trimmedResult)) continue;

                std::vector<int> refs;
                for (int idx : combo)
                    refs.push_back(proofLines[idx].lineNumber);

                proofLines.push_back({
                    static_cast<int>(proofLines.size()) + 1,
                    trimmedResult,
                    rule.name,
                    refs,
                    currentIndent
                });

                seen.insert(trimmedResult);
                progress = true;

                // Direct match with consequent?
                if (trimmedResult == normalizeConnectives(consequent)) {
                    int qedLine = static_cast<int>(proofLines.size());
                    // Close the subproof (manually adjust indent)
                    if (!showStack.empty()) {
                        showStack.pop_back();
                        currentIndent = showStack.empty() ? 0 : showStack.back();
                    }

                    if (proofLines.size() > 5000) {
                        std::cerr << "[ERROR] Proof line explosion (>5000). Aborting CD.\n";
                        cdDepth--;
                        return false;
                    }


                    // Push the actual implication line as conclusion of the CD
                    proofLines.push_back({
                        static_cast<int>(proofLines.size()) + 1,
                        implicationStr,
                        "CD",
                        {qedLine},
                        currentIndent
                    });

                    cdDepth--;
                    return true;
                }

                // Consequent is an implication? Try CD on it.
                auto inner = splitImplication(consequent);
                if (inner && tryConditionalDerivation(consequent, attempted)) {
                    int qedLine = static_cast<int>(proofLines.size());
                    // Pop subproof indent level (manual version of endSubproof)
                    if (!showStack.empty()) {
                        showStack.pop_back();
                        currentIndent = showStack.empty() ? 0 : showStack.back();
                    }

                    // Push final implication as actual proof line
                    proofLines.push_back({
                        static_cast<int>(proofLines.size()) + 1,
                        implicationStr,
                        "CD",
                        {qedLine},
                        currentIndent
                    });
                    cdDepth--;
                    return true;
                }
            }
        }
    }

    cdDepth--;
    return false;
}

bool ProofSolver::tryDirectDerivation(const std::string& goal) {
    std::string normalizedGoal = normalizeConnectives(trim(goal));

    for (const auto& stmt : proofLines) {
        if (normalizeConnectives(trim(stmt.expression)) == normalizedGoal &&
            stmt.justification != "" &&  // skip "Show:" line
            stmt.justification != "PR") // usually only assumptions or derived lines
        {
            endSubproof("DD", {stmt.lineNumber});
            return true;
        }
    }

    return false;
}

void ProofSolver::startSubproof(const std::string& formula) {
    int showLineNum = static_cast<int>(proofLines.size()) + 1;

    // Insert "Show: φ"
    proofLines.push_back({
        showLineNum,
        "Show: " + formula,
        "",
        {},
        currentIndent + 1
    });

    // Push subproof context
    showStack.push_back(currentIndent + 1);
    currentIndent++;

    // Insert assumption line φ :AS
    proofLines.push_back({
        showLineNum + 1,
        formula,
        "AS",
        {},
        currentIndent
    });
}

void ProofSolver::endSubproof(const std::string& rule, const std::vector<int>& refs) {
    int qedLineNum = static_cast<int>(proofLines.size()) + 1;

    // Insert QED line
    proofLines.push_back({
        qedLineNum,
        "",          // no expression for QED, only justification
        rule,        // e.g., CD, DD, ID
        refs,
        currentIndent  // same indent as containing proof
    });

    // Pop the subproof
    if (!showStack.empty()) {
        showStack.pop_back();
        currentIndent = showStack.empty() ? 0 : showStack.back();
    }
}

void ProofSolver::displayProof() const {
    std::cout << "=== Proof Steps ===\n";
    for (const auto& stmt : proofLines) {
        std::string indent(stmt.indentLevel * 3, ' ');
        std::string expr = beautify ? beautifyConnectives(stmt.expression) : stmt.expression;

        if (stmt.lineNumber == 1) {
            std::cout << stmt.lineNumber << ". " << expr << "\n";
        } else {
            std::cout << indent << stmt.lineNumber << ".  " << expr;
            if (!stmt.justification.empty()) {
                std::cout << "    :" << stmt.justification;
            }

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

bool ProofSolver::wasConclusionDerived() const {
    std::string goal = normalizeConnectives(trim(conclusion));
    for (const auto& stmt : proofLines) {
        if (normalizeConnectives(trim(stmt.expression)) == goal)
            return true;
    }
    return false;
}

const std::vector<Statement>& ProofSolver::getProofLines() const {
    return proofLines;
}


void ProofSolver::setInput(const std::string& premisesStr, const std::string& conclusionStr) {
    premises.clear();
    std::stringstream ss(premisesStr);
    std::string item;
    while (std::getline(ss, item, ',')) {
        premises.push_back(normalizeConnectives(trim(item)));
    }
    conclusion = normalizeConnectives(trim(conclusionStr));
}

void ProofSolver::enableBeautify(bool enable) {
    beautify = enable;
}