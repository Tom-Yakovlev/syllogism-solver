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

// In ProofSolver.cpp
#include "ProofSolver.h"
#include "Utils.h"
#include "Rules.h"
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <unordered_map>

// === Backward chaining implementation ===
bool ProofSolver::prove(const std::string& goal) {
    std::string normGoal = normalizeConnectives(trim(goal));

    if (goal.empty()) {
        std::cerr << "[ERROR] Empty goal passed to prove()\n";
        return false;
    }

    // Prevent infinite loops
    if (visitedGoals.count(normGoal)) {
        return false;
    }
    visitedGoals.insert(normGoal);

    // 1. Already proven?
    for (const auto& stmt : proofLines) {
        if (normalizeConnectives(trim(stmt.expression)) == normGoal)
            return true;
    }

    // 2. Is a premise?
    for (const auto& prem : premises) {
        if (normalizeConnectives(prem) == normGoal) {
            proofLines.push_back({
                static_cast<int>(proofLines.size()) + 1,
                prem,
                "PR",
                {},
                currentIndent
            });
            return true;
        }
    }

    // 3. Try applying rules backward
    for (const auto& rule : rules) {
        auto guesses = guessPremisesFor(rule, normGoal);

        for (const auto& guess : guesses) {
            bool allProven = true;
            std::vector<int> refs;

            for (const auto& needed : guess) {
                if (!prove(needed)) {
                    allProven = false;
                    break;
                }

                int lineNum = getLineNumberFor(needed);
                if (lineNum == -1) {
                    std::cerr << "[ERROR] Could not find line for proven statement: " << needed << "\n";
                    allProven = false;
                    break;
                }

                refs.push_back(lineNum);
            }

            if (allProven) {
                proofLines.push_back({
                    static_cast<int>(proofLines.size()) + 1,
                    normGoal,
                    rule.name,
                    refs,
                    currentIndent
                });
                return true;
            }
        }
    }

    return false; // No derivation found
}

int ProofSolver::getLineNumberFor(const std::string& expr) const {
    std::string norm = normalizeConnectives(trim(expr));
    for (const auto& stmt : proofLines) {
        if (normalizeConnectives(stmt.expression) == norm) {
            return stmt.lineNumber;
        }
    }
    return -1;
}


std::vector<std::vector<std::string>> ProofSolver::guessPremisesFor(const Rule& rule, const std::string& goal) {
    std::vector<std::vector<std::string>> guesses;
    std::string g = normalizeConnectives(trim(goal));

    if (rule.name == "MP") {
        auto imp = splitImplication(g);
        if (imp) guesses.push_back({imp->first, g});
    }
    else if (rule.name == "MT") {
        if (g.rfind("~", 0) == 0) {
            std::string not_phi = g.substr(1);
            if (not_phi.empty() || not_phi.size() > 50) return guesses; // avoid runaway recursion
            guesses.push_back({"~X", not_phi + "->X"});
        }
    }
    else if (rule.name == "MTP") {
        size_t vpos = g.find("v");
        if (vpos != std::string::npos) {
            std::string left = g.substr(0, vpos);
            std::string right = g.substr(vpos + 1);
            guesses.push_back({left + "v" + right, "~" + left});
            guesses.push_back({left + "v" + right, "~" + right});
        }
    }
    else if (rule.name == "DNE") {
        guesses.push_back({"~~" + g});
    }
    else if (rule.name == "DNI") {
        if (g.rfind("~~", 0) == 0)
            guesses.push_back({g.substr(2)});
    }
    else if (rule.name == "S") {
        guesses.push_back({g + "^X"});
        guesses.push_back({"X^" + g});
    }
    else if (rule.name == "ADJ") {
        size_t pos = g.find("^");
        if (pos != std::string::npos) {
            std::string left = g.substr(0, pos);
            std::string right = g.substr(pos + 1);
            guesses.push_back({left, right});
        }
    }
    else if (rule.name == "ADD") {
        guesses.push_back({g.substr(0, g.find("v"))});
    }
    else if (rule.name == "CB") {
        auto imp = splitImplication(g);
        if (imp) guesses.push_back({imp->first + "->" + imp->second, imp->second + "->" + imp->first});
    }
    else if (rule.name == "BC") {
        auto imp = splitImplication(g);
        if (imp) guesses.push_back({imp->first + "<->" + imp->second, g});
    }
    else if (rule.name == "D-CPO") {
        auto imp = splitImplication(g);
        if (imp && imp->first.rfind("~", 0) == 0 && imp->second.rfind("~", 0) == 0) {
            std::string psi = imp->first.substr(1);
            std::string phi = imp->second.substr(1);
            guesses.push_back({phi + "->" + psi});
        }
    }
    else if (rule.name == "D-CPT") {
        auto imp = splitImplication(g);
        if (imp) {
            guesses.push_back({"~" + imp->second + "->~" + imp->first});
        }
    }
    else if (rule.name == "D-DIL") {
        guesses.push_back({"~φ->" + g, "φ->" + g});
    }
    else if (rule.name == "D-CM") {
        guesses.push_back({"~" + g + "->" + g});
    }
    else if (rule.name == "D-EFQ") {
        guesses.push_back({g, "~" + g});
        guesses.push_back({"~" + g, g});
    }
    else if (rule.name == "D-DMO") {
        guesses.push_back({"~(" + g + ")<->(~P^~Q)"});
    }
    else if (rule.name == "D-DMT") {
        guesses.push_back({"~(" + g + ")<->(~P v ~Q)"});
    }
    else if (rule.name == "D-SDMO") {
        guesses.push_back({g + "<->~(~P v ~Q)"});
    }
    else if (rule.name == "D-SDMT") {
        guesses.push_back({"(" + g + ")<->~(~P^~Q)"});
    }
    else if (rule.name == "D-NC") {
        guesses.push_back({"~(P->Q)<->(P^~Q)"});
    }

    return guesses;
}

// === Replace your solve() with this ===
void ProofSolver::solve() {
    rules = getAllRules();
    proofLines.clear();
    visitedGoals.clear(); // clear cache

    proofLines.push_back({
        1,
        "Show: " + trim(conclusion),
        "",
        {},
        0
    });
    showStack.push_back(0);
    currentIndent = 0;

    std::cout << "[DEBUG] Premises loaded: ";
    for (const auto& p : premises) std::cout << "[" << p << "] ";
    std::cout << "\n";
    std::cout << "[DEBUG] Conclusion: " << conclusion << "\n";

    if (prove(conclusion)) {
        displayProof();
    } else {
        std::cerr << "\033[31m[FAILED TO PROVE]\033[0m: " << conclusion << "\n";
    }
}


// Helper Function for solver()
bool ProofSolver::tryConditionalDerivation(const std::string& implicationStr) {
    static int cdDepth = 0;
    cdDepth++;
    if (cdDepth > 10) {
        std::cerr << "[ERROR] Maximum CD recursion depth exceeded.\n";
        cdDepth--;
        return false;
    }

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
                if (inner && tryConditionalDerivation(consequent)) {
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
        auto trimmed = normalizeConnectives(trim(item));
        if (!trimmed.empty())
            premises.push_back(trimmed);
    }
    conclusion = normalizeConnectives(trim(conclusionStr));

    std::cout << "[DEBUG] setInput parsed premises: ";
    for (const auto& p : premises) std::cout << "[" << p << "] ";
    std::cout << "\n[DEBUG] Conclusion: " << conclusion << "\n";
}


void ProofSolver::enableBeautify(bool enable) {
    beautify = enable;
}
