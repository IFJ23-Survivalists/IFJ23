/**
 * @file grammar.h
 * @brief Rule and grammar definitions for LL-table generation
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#pragma once
#include <map>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdarg>
extern "C" {
#include "../scanner.h"    // Token
#include "../cfl.h"
}


static const char* NTERM_NAMES[] = {
    "<statementList>",
    "<statement>",
    "<params>",
    "<params_n>",
    "<returnExpr>",
    "<ifCondition>",
    "<else>",
    "<assignType>",
    "<assignExpr>",
    "<expr>",
};

inline std::string_view to_string(NTerm t) { return std::string_view(NTERM_NAMES[(int)t]); }

static const char* TOKENTYPE_NAMES[] = {
    "$",
    "EOL",      // FIXME: Make sure this is not a Whitespace in the future.

    "{",
    "}",
    "(",
    ")",
    ":",
    "->",
    "=",
    ",",

    "if",
    "else",
    "let",
    "var",
    "while",
    "func",
    "return",

    "Data",
    "DataType",
    "Op",
    "Keyword",
    "ID",
};

inline std::string_view to_string(TokenType t) { return std::string_view(TOKENTYPE_NAMES[t]); }

inline bool operator==(const Terminal& lhs, const Terminal& rhs) { return lhs.tok == rhs.tok; }

inline bool operator!=(const Terminal& lhs, const Terminal& rhs) { return lhs.tok != rhs.tok; }

class TerminalIterator {
    int val;
public:
    static TerminalIterator begin(int val = 0);
    static TerminalIterator end();
    TerminalIterator operator+(int val);
    TerminalIterator operator-(int val);
    TerminalIterator& operator+=(int val);
    TerminalIterator& operator-=(int val);
    TerminalIterator& operator++();
    TerminalIterator& operator--();
    bool operator==(const TerminalIterator& rhs) const;
    bool operator!=(const TerminalIterator& rhs) const;
    Terminal operator*();
    Terminal operator->();
};

template<>
struct std::hash<Terminal> {
    inline size_t operator()(const Terminal& t) const {
        return std::hash<TokenType>{}(t.tok);
    }
};

inline bool operator==(const Symbol& lhs, const Symbol& rhs) {
    if (lhs.is_term != rhs.is_term)
        return false;
    return lhs.is_term ? lhs.term == rhs.term : lhs.nterm == rhs.nterm;
}

inline bool operator!=(const Symbol& lhs, const Symbol& rhs) {
    return !(lhs == rhs);
}

inline std::string_view to_string(Terminal t) { return to_string(t.tok); }
inline std::string_view to_string(Symbol s) {
    return s.is_term ? to_string(s.term) : to_string(s.nterm);
}

/// Represents all rule a given with NTerm on left side
struct Rule {
    NTerm lhs;                ///< Left-hand side
    std::vector<Symbol> rhs;
    std::unordered_set<Terminal> predict;

    inline bool eps() { return rhs.size() == 0; }
};

struct Grammar {
    std::map<NTerm, std::vector<Rule>> rules;

    /**
     * @brief Create a rule from given tokens
     * @param lhs Left-hand side of the rule
     * @param rhsfmt Format of the right-hand side.
     *      n -> NTerm,
     *      a -> Automatic from char,
     *      t -> TokenType,
     * @pre Will set Error_Internal on error.
     * @return Created rule. This rule needs to be freed by `destroy_rule()` function.
     */
    void add_rule(NTerm nterm, const char* rhsfmt, ...);
    void print();
    Grammar();
    ~Grammar();
};

