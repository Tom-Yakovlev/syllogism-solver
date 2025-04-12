#include "Rules.h"
#include "Utils.h"
#include <regex>
#include <optional>

// Utility: Checks if string is in the form "A->B" and returns pair {A, B}
std::optional<std::pair<std::string, std::string>> splitImplication(const std::string& expr) {
    size_t arrow = expr.find("->");
    if (arrow != std::string::npos) {
        std::string left = trim(expr.substr(0, arrow));
        std::string right = trim(expr.substr(arrow + 2));
        return std::make_pair(left, right);
    }
    return std::nullopt;
}

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


std::vector<Rule> getAllRules() {
    return {
        makeMP(),
        makeMT(),
        makeDNE(),
        makeDNI()
        // Add more rules here
    };
}
