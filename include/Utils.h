#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <algorithm>
#include <cctype>

// Trim helper
inline std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

// Normalize logical connectives to canonical forms
inline std::string normalizeConnectives(const std::string& raw) {
    std::string s = trim(raw);

    // Do all replacements using string-safe substitutions
    // Order matters: longest substrings first
    const std::vector<std::pair<std::string, std::string>> replacements = {
        {"<=>", "<->"},
        {"<->", "<->"},
        {"=>", "->"},
        {"->", "->"},

        {"\\/", "v"},
        {"or", "v"},
        {"|", "v"},

        {"/\\", "^"},
        {"and", "^"},
        {"&", "^"},

        {"not", "~"},
    };

    for (const auto& [from, to] : replacements) {
        size_t pos = 0;
        while ((pos = s.find(from, pos)) != std::string::npos) {
            s.replace(pos, from.length(), to);
            pos += to.length();  // advance to avoid infinite loop
        }
    }

    return s;
}


inline std::string beautifyConnectives(const std::string& raw) {
    std::string s = raw;

    const std::vector<std::pair<std::string, std::string>> replacements = {
        {"<->", "↔︎"},
        {"->", "→"},
        {"^",  "∧"},
        {"v",  "∨"},
        {"~",  "¬"}
    };

    for (const auto& [from, to] : replacements) {
        size_t pos = 0;
        while ((pos = s.find(from, pos)) != std::string::npos) {
            s.replace(pos, from.length(), to);
            pos += to.length();
        }
    }

    return s;
}

inline std::optional<std::pair<std::string, std::string>> splitImplication(const std::string& expr) {
    size_t pos = expr.find("->");
    if (pos != std::string::npos) {
        std::string lhs = trim(expr.substr(0, pos));
        std::string rhs = trim(expr.substr(pos + 2));
        return std::make_pair(lhs, rhs);
    }
    return std::nullopt;
}


#endif // UTILS_H
