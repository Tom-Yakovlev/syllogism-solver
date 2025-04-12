#include "Rules.h"
#include "Utils.h"
#include <regex>
#include <optional>

// Modus Ponens (MP): From A and A->B, conclude B
Rule makeMP() {
    return {
        "MP",
        2,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string a = trim(premises[0]);
            std::string b = trim(premises[1]);

            // Try both combinations
            auto tryMP = [&](const std::string& phi, const std::string& implication) -> std::optional<std::string> {
                auto maybe = splitImplication(implication);
                if (maybe && trim(maybe->first) == phi) {
                    return trim(maybe->second);
                }
                return std::nullopt;
            };

            if (auto res = tryMP(a, b)) return res;
            if (auto res = tryMP(b, a)) return res;

            return std::nullopt;
        }
    };
}

// Modus Tollens (MT): From ~ψ and φ→ψ, conclude ~φ
Rule makeMT() {
    return {
        "MT",
        2,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string a = trim(premises[0]);
            std::string b = trim(premises[1]);

            auto tryMT = [&](const std::string& notPsi, const std::string& imp) -> std::optional<std::string> {
                if (notPsi.rfind("~", 0) == 0) {
                    std::string psi = trim(notPsi.substr(1));
                    auto maybeImp = splitImplication(imp);
                    if (maybeImp && trim(maybeImp->second) == psi) {
                        return "~" + trim(maybeImp->first);
                    }
                }
                return std::nullopt;
            };

            if (auto res = tryMT(a, b)) return res;
            if (auto res = tryMT(b, a)) return res;
            return std::nullopt;
        }
    };
}

Rule makeDNE() {
    return {
        "DNE",
        1,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string expr = trim(premises[0]);

            if (expr.rfind("~~", 0) == 0) {
                return trim(expr.substr(2)); // remove the two leading negations
            }

            return std::nullopt;
        }
    };
}

Rule makeDNI() {
    return {
        "DNI",
        1,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string expr = trim(premises[0]);
            return "~~" + expr;
        }
    };
}

Rule makeS() {
    return {
        "S",
        1,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string expr = trim(premises[0]);

            size_t andPos = expr.find("^");
            if (andPos != std::string::npos) {
                std::string left = trim(expr.substr(0, andPos));
                std::string right = trim(expr.substr(andPos + 1));

                static bool toggle = false;
                toggle = !toggle;
                return toggle ? left : right;
            }

            return std::nullopt;
        }
    };
}

Rule makeADJ() {
    return {
        "ADJ",
        2,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            if (premises.size() != 2) return std::nullopt;

            std::string a = trim(premises[0]);
            std::string b = trim(premises[1]);

            if (a == b) return std::nullopt; // Don't introduce redundancy like "P∧P"

            return a + "^" + b;
        }
    };
}

Rule makeMTP() {
    return {
        "MTP",
        2,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string a = trim(premises[0]);
            std::string b = trim(premises[1]);

            auto tryMTP = [](const std::string& disj, const std::string& negated) -> std::optional<std::string> {
                if (disj.find("|") == std::string::npos || negated.rfind("~", 0) != 0)
                    return std::nullopt;

                size_t barPos = disj.find("|");
                std::string left = trim(disj.substr(0, barPos));
                std::string right = trim(disj.substr(barPos + 1));
                std::string negTerm = trim(negated.substr(1));  // remove ~

                if (negTerm == left) return right;
                if (negTerm == right) return left;

                return std::nullopt;
            };

            if (auto res = tryMTP(a, b)) return res;
            if (auto res = tryMTP(b, a)) return res;

            return std::nullopt;
        }
    };
}

Rule makeADD() {
    return {
        "ADD",
        1,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string expr = trim(premises[0]);

            // We can't infer what ψ is, so we only support ADD for generating disjunctions
            // like φ ∨ ψ, where φ is known and ψ is arbitrary.
            // For simplicity, just add a fixed placeholder for the second disjunct.

            return expr + "|" + "ψ";  // You can later modify this to allow ψ to be any symbol if user input or chaining is supported.
        }
    };
}

Rule makeBC() {
    return {
        "BC",
        2,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            if (premises.size() != 2) return std::nullopt;

            std::string biconditional = trim(premises[0]);
            std::string implication = trim(premises[1]);

            size_t bicondPos = biconditional.find("<->");
            size_t implPos = implication.find("->");

            if (bicondPos == std::string::npos || implPos == std::string::npos)
                return std::nullopt;

            std::string lhs = trim(biconditional.substr(0, bicondPos));
            std::string rhs = trim(biconditional.substr(bicondPos + 3));

            std::string antecedent = trim(implication.substr(0, implPos));
            std::string consequent = trim(implication.substr(implPos + 2));

            if ((antecedent == lhs && consequent == rhs) ||
                (antecedent == rhs && consequent == lhs)) {
                return implication;
            }

            return std::nullopt;
        }
    };
}

Rule makeCB() {
    return {
        "CB",
        2,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            if (premises.size() != 2) return std::nullopt;

            auto imp1 = splitImplication(premises[0]);
            auto imp2 = splitImplication(premises[1]);

            if (!imp1 || !imp2) return std::nullopt;

            std::string a1 = trim(imp1->first);
            std::string b1 = trim(imp1->second);
            std::string a2 = trim(imp2->first);
            std::string b2 = trim(imp2->second);

            if (a1 == b2 && b1 == a2) {
                return a1 + "<->" + b1;
            }

            return std::nullopt;
        }
    };
}

// Derived Rule: Hypothetical Syllogism (D-HS)
// From (ψ → χ) and (φ → ψ), derive (φ → χ)
Rule makeD_HS() {
    return {
        "D-HS",
        2,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            auto imp1 = splitImplication(trim(premises[0]));
            auto imp2 = splitImplication(trim(premises[1]));

            if (imp1 && imp2) {
                // Try both orderings: imp1 then imp2, or imp2 then imp1
                if (normalizeConnectives(imp2->second) == normalizeConnectives(imp1->first)) {
                    return imp2->first + "->" + imp1->second;
                } else if (normalizeConnectives(imp1->second) == normalizeConnectives(imp2->first)) {
                    return imp1->first + "->" + imp2->second;
                }
            }
            return std::nullopt;
        }
    };
}

// Derived Rule: Material Conditional Construction (D-MCC)
// From φ, derive (ψ → φ) for arbitrary ψ (default ψ = X)
Rule makeD_MCC() {
    return {
        "D-MCC",
        1,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string phi = trim(premises[0]);

            // Use a generic placeholder for ψ — user may later customize this
            std::string psi = "X";

            return psi + "->" + phi;
        }
    };
}

// Derived Rule: Material Conditional from Negated Antecedent (D-MCNA)
// From ~φ, derive (φ → ψ) for arbitrary ψ (default ψ = X)
Rule makeD_MCNA() {
    return {
        "D-MCNA",
        1,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string not_phi = trim(premises[0]);

            if (not_phi.rfind("~", 0) != 0) return std::nullopt;

            std::string phi = not_phi.substr(1);  // remove the ~
            std::string psi = "X";  // placeholder or fresh variable

            return phi + "->" + psi;
        }
    };
}

// Derived Rule: Contrapositive (D-CPO)
// From (φ → ψ), derive (~ψ → ~φ)
Rule makeD_CPO() {
    return {
        "D-CPO",
        1,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string implication = trim(premises[0]);

            auto maybe = splitImplication(implication);
            if (!maybe) return std::nullopt;

            std::string phi = trim(maybe->first);
            std::string psi = trim(maybe->second);

            return "~" + psi + "->~" + phi;
        }
    };
}

// Derived Rule: Converse Contrapositive (D-CPT)
// From (~φ → ~ψ), derive (ψ → φ)
Rule makeD_CPT() {
    return {
        "D-CPT",
        1,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string implication = trim(premises[0]);

            auto maybe = splitImplication(implication);
            if (!maybe) return std::nullopt;

            std::string not_phi = trim(maybe->first);
            std::string not_psi = trim(maybe->second);

            if (not_phi.rfind("~", 0) != 0 || not_psi.rfind("~", 0) != 0)
                return std::nullopt;

            std::string phi = not_phi.substr(1);
            std::string psi = not_psi.substr(1);

            return trim(psi) + "->" + trim(phi);
        }
    };
}

// Derived Rule: Disjunction via Implication Law (D-DIL)
// From (~φ → ψ), (φ → ψ), derive ψ
Rule makeD_DIL() {
    return {
        "D-DIL",
        2,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string a = trim(premises[0]);
            std::string b = trim(premises[1]);

            auto tryDIL = [](const std::string& first, const std::string& second) -> std::optional<std::string> {
                auto imp1 = splitImplication(first);
                auto imp2 = splitImplication(second);
                if (!imp1 || !imp2) return std::nullopt;

                std::string ant1 = trim(imp1->first);
                std::string cons1 = trim(imp1->second);
                std::string ant2 = trim(imp2->first);
                std::string cons2 = trim(imp2->second);

                if (ant1.rfind("~", 0) == 0) {
                    std::string phi1 = ant1.substr(1);
                    if (ant2 == phi1 && cons1 == cons2) {
                        return cons1;
                    }
                }

                return std::nullopt;
            };

            if (auto res = tryDIL(a, b)) return res;
            if (auto res = tryDIL(b, a)) return res;
            return std::nullopt;
        }
    };
}

// Derived Rule: Contradiction Method (D-CM)
// From (~φ → φ), derive φ
Rule makeD_CM() {
    return {
        "D-CM",
        1,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string imp = trim(premises[0]);
            auto maybe = splitImplication(imp);
            if (!maybe) return std::nullopt;

            std::string antecedent = trim(maybe->first);
            std::string consequent = trim(maybe->second);

            if (antecedent.rfind("~", 0) == 0) {
                std::string phi = antecedent.substr(1);
                if (phi == consequent) {
                    return phi;
                }
            }

            return std::nullopt;
        }
    };
}

// Derived Rule: Ex Falso Quodlibet (D-EFQ)
// From φ and ~φ, derive any formula ψ (we'll pick a canonical one for now)
Rule makeD_EFQ() {
    return {
        "D-EFQ",
        2,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string a = trim(premises[0]);
            std::string b = trim(premises[1]);

            // Check for φ and ¬φ in any order
            if (a.rfind("~", 0) == 0 && a.substr(1) == b)
                return "R";  // pick arbitrary formula R as placeholder
            if (b.rfind("~", 0) == 0 && b.substr(1) == a)
                return "R";

            return std::nullopt;
        }
    };
}

// Second De Morgan One
// From (φ ^ ψ) <-> ~(~φ v ~ψ) or vice versa
Rule makeD_SDMO() {
    return {
        "D-SDMO",
        1,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string expr = normalizeConnectives(trim(premises[0]));
            size_t iffPos = expr.find("<->");
            if (iffPos == std::string::npos) return std::nullopt;

            std::string left = trim(expr.substr(0, iffPos));
            std::string right = trim(expr.substr(iffPos + 3));

            // Check one direction: (A ^ B) <-> ~(~A v ~B)
            auto isDeMorganSDMO = [](const std::string& a, const std::string& b) -> bool {
                if (a.find("^") == std::string::npos) return false;
                if (b.rfind("~(", 0) != 0 || b.back() != ')') return false;

                std::string inner = b.substr(2, b.size() - 3); // strip outer ~(...)
                size_t vPos = inner.find("v");
                if (vPos == std::string::npos) return false;

                std::string left = trim(inner.substr(0, vPos));
                std::string right = trim(inner.substr(vPos + 1));

                return (left.rfind("~", 0) == 0 && right.rfind("~", 0) == 0);
            };

            if (isDeMorganSDMO(left, right) || isDeMorganSDMO(right, left)) {
                return expr;
            }

            return std::nullopt;
        }
    };
}

// First De Morgan One
// From ~(φ v ψ) <-> (~φ ^ ~ψ) or vice versa
Rule makeD_DMO() {
    return {
        "D-DMO",
        1,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string expr = normalizeConnectives(trim(premises[0]));
            size_t iffPos = expr.find("<->");
            if (iffPos == std::string::npos) return std::nullopt;

            std::string left = trim(expr.substr(0, iffPos));
            std::string right = trim(expr.substr(iffPos + 3));

            // Matcher: ~(A v B) <-> (~A ^ ~B)
            auto isDeMorganDMO = [](const std::string& a, const std::string& b) -> bool {
                // Check if a is ~(...v...)
                if (a.rfind("~(", 0) != 0 || a.back() != ')') return false;
                std::string innerA = a.substr(2, a.size() - 3);
                size_t vPos = innerA.find("v");
                if (vPos == std::string::npos) return false;
                std::string left = trim(innerA.substr(0, vPos));
                std::string right = trim(innerA.substr(vPos + 1));
                if (left.rfind("~", 0) != 0 || right.rfind("~", 0) != 0) return false;

                // Check if b is conjunction
                size_t andPos = b.find("^");
                if (andPos == std::string::npos) return false;
                std::string b1 = trim(b.substr(0, andPos));
                std::string b2 = trim(b.substr(andPos + 1));
                return (b1 == left && b2 == right);
            };

            if (isDeMorganDMO(left, right) || isDeMorganDMO(right, left)) {
                return expr;
            }

            return std::nullopt;
        }
    };
}

// First De Morgan Two
// From ~(φ ^ ψ) <-> (~φ v ~ψ) or vice versa
Rule makeD_DMT() {
    return {
        "D-DMT",
        1,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string expr = normalizeConnectives(trim(premises[0]));
            size_t iffPos = expr.find("<->");
            if (iffPos == std::string::npos) return std::nullopt;

            std::string left = trim(expr.substr(0, iffPos));
            std::string right = trim(expr.substr(iffPos + 3));

            // Matcher: ~(A ^ B) <-> (~A v ~B)
            auto isDMT = [](const std::string& a, const std::string& b) -> bool {
                // Left side must be ~(...) with ^ inside
                if (a.rfind("~(", 0) != 0 || a.back() != ')') return false;
                std::string inner = a.substr(2, a.size() - 3);
                size_t andPos = inner.find("^");
                if (andPos == std::string::npos) return false;
                std::string leftConj = trim(inner.substr(0, andPos));
                std::string rightConj = trim(inner.substr(andPos + 1));
                if (leftConj.rfind("~", 0) != 0 || rightConj.rfind("~", 0) != 0) return false;

                // Right side must be a disjunction of those same negated parts
                size_t orPos = b.find("v");
                if (orPos == std::string::npos) return false;
                std::string b1 = trim(b.substr(0, orPos));
                std::string b2 = trim(b.substr(orPos + 1));
                return (b1 == leftConj && b2 == rightConj);
            };

            if (isDMT(left, right) || isDMT(right, left)) {
                return expr;
            }

            return std::nullopt;
        }
    };
}

// Second De Morgan Two
// (φ v ψ) <-> ~(~φ ^ ~ψ)
Rule makeD_SDMT() {
    return {
        "D-SDMT",
        1,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string expr = normalizeConnectives(trim(premises[0]));
            size_t iffPos = expr.find("<->");
            if (iffPos == std::string::npos) return std::nullopt;

            std::string left = trim(expr.substr(0, iffPos));
            std::string right = trim(expr.substr(iffPos + 3));

            // Matcher: (A v B) <-> ~(~A ^ ~B)
            auto isSDMT = [](const std::string& a, const std::string& b) -> bool {
                // First side must be (A v B)
                size_t orPos = a.find("v");
                if (a.front() != '(' || a.back() != ')' || orPos == std::string::npos)
                    return false;
                std::string leftOr = trim(a.substr(1, orPos - 1));
                std::string rightOr = trim(a.substr(orPos + 1, a.size() - orPos - 2));

                // Second side must be ~(~A ^ ~B)
                if (b.rfind("~(", 0) != 0 || b.back() != ')') return false;
                std::string inner = b.substr(2, b.size() - 3);
                size_t andPos = inner.find("^");
                if (andPos == std::string::npos) return false;
                std::string leftConj = trim(inner.substr(0, andPos));
                std::string rightConj = trim(inner.substr(andPos + 1));
                if (leftConj.rfind("~", 0) != 0 || rightConj.rfind("~", 0) != 0) return false;

                std::string negLeftOr = "~" + leftOr;
                std::string negRightOr = "~" + rightOr;
                return (leftConj == negLeftOr && rightConj == negRightOr);
            };

            if (isSDMT(left, right) || isSDMT(right, left)) {
                return expr;
            }

            return std::nullopt;
        }
    };
}

// Proof by cases
// From:(φ → χ),(φ ∨ ψ),(ψ → χ)
// Derive: χ
Rule makeD_PBC() {
    return {
        "D-PBC",
        3,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string a = trim(premises[0]);
            std::string b = trim(premises[1]);
            std::string c = trim(premises[2]);

            // Try all permutations of the premises to find the pattern
            std::vector<std::string> perms[] = {
                {a, b, c},
                {a, c, b},
                {b, a, c},
                {b, c, a},
                {c, a, b},
                {c, b, a}
            };

            for (const auto& p : perms) {
                auto imp1 = splitImplication(p[0]);
                auto disj  = p[1];
                auto imp2 = splitImplication(p[2]);

                if (imp1 && imp2 &&
                    normalizeConnectives(disj).find("v") != std::string::npos) {
                    
                    std::string phi1 = trim(imp1->first);
                    std::string chi1 = trim(imp1->second);

                    std::string phi2 = trim(imp2->first);
                    std::string chi2 = trim(imp2->second);

                    // Make sure both conditionals conclude the same thing
                    if (chi1 != chi2)
                        continue;

                    // Check that the disjunction contains both antecedents
                    std::string disjNorm = normalizeConnectives(disj);
                    if ((disjNorm == (phi1 + "v" + phi2)) || (disjNorm == (phi2 + "v" + phi1))) {
                        return chi1;
                    }
                }
            }

            return std::nullopt;
        }
    };
}

Rule makeD_NC() {
    return {
        "D-NC",
        1,
        [](const std::vector<std::string>& premises) -> std::optional<std::string> {
            std::string expr = trim(premises[0]);

            const std::string form1 = "~(\u03c6->\u03c8)<->(\u03c6^~\u03c8)";
            const std::string form2 = "(\u03c6^~\u03c8)<->~(\u03c6->\u03c8)";

            if (expr == form1 || expr == form2)
                return expr;

            return std::nullopt;
        }
    };
}

std::vector<Rule> getAllRules() {
    return {
        makeMP(),
        makeMT(),
        makeDNE(),
        makeDNI(),
        makeS(),
        makeADJ(),
        makeMTP(),
        makeADD(),
        makeBC(),
        makeCB(),
        makeD_HS(),
        makeD_MCC(),
        makeD_MCNA(),
        makeD_CPO(),
        makeD_CPT(),
        makeD_DIL(),
        makeD_CM(),
        makeD_EFQ(),
        makeD_SDMO(),
        makeD_DMO(),
        makeD_DMT(),
        makeD_SDMT(),
        makeD_PBC(),
        makeD_NC()
    };
}
