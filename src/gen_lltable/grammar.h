#pragma once
#include <map>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdarg>
extern "C" {
#include "../scanner.h"    // Token
}

const int KW_COUNT = Keyword_Return + 1;
const int TOK_COUNT = Token_Identifier + 1;

 /// All possible non-terminals
enum class NTerm : int{
    StatementList = 0,
    Statement,
    Params,
    Params_n,
    ArgumentLabel,
    ReturnExpr,
    IfCondition,
    Else,
    AssignType,
    AssignExpr,
    Expr,
    ExprInner,
    FunctionCall,
    FunctionCallParams,
    FunctionCallParams_n,
    ParamName,

    /// Number of non-terminals in this enum.
    count
};

static const char* NTERM_NAMES[] = {
    "<statementList>",
    "<statement>",
    "<params>",
    "<params_n>",
    "<argumentLabel>",
    "<returnExpr>",
    "<ifCondition>",
    "<else>",
    "<assignType>",
    "<assignExpr>",
    "<expr>",
    "<exprInner>",
    "<functionCall>",
    "<functionCallParams>",
    "<functionCallParams_n>",
    "<paramName>",
};

inline std::string_view to_string(NTerm t) { return std::string_view(NTERM_NAMES[(int)t]); }

static const char* KEYWORD_NAMES[] = {
    "if",
    "else",
    "let",
    "var",
    "while",
    "func",
    "return",
};

inline std::string_view to_string(Keyword k) { return std::string_view(KEYWORD_NAMES[k]); }

static const char* TOKENTYPE_NAMES[] = {
    "$",
    "Whitespace",
    "{",
    "}",
    "(",
    ")",
    "@",
    ":",
    "->",
    "=",
    ",",
    "Data",
    "DataType",
    "Op",
    "Keyword",
    "ID",
};

inline std::string_view to_string(TokenType t) { return std::string_view(TOKENTYPE_NAMES[t]); }

struct Terminal {
    bool is_kw{ false };
    union {
        Keyword kw;
        TokenType tok;
    };

    bool operator==(const Terminal& rhs) const {
        if (is_kw != rhs.is_kw)
            return false;
        return is_kw ? kw == rhs.kw : tok == rhs.tok;
    }
    bool operator!=(const Terminal& rhs) const {
        return !(*this == rhs);
    }
};

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

    static Terminal val_to_term(int val);
    static int term_to_val(const Terminal& t);
};

template<>
struct std::hash<Terminal> {
    size_t operator()(const Terminal& t) const {
        return t.is_kw ? std::hash<Keyword>{}(t.kw) : std::hash<TokenType>{}(t.tok);
    }
};

/// Represents a right-hand side of a grammar rule.
struct Symbol {
    bool is_term{ false };
    union {
        Terminal term;
        NTerm nterm;
    };

    Symbol() {}

    bool operator==(const Symbol& rhs) const {
        if (is_term != rhs.is_term)
            return false;
        return is_term ? term == rhs.term : nterm == rhs.nterm;
    }
    bool operator!=(const Symbol& rhs) const {
        return !((*this) == rhs);
    }
};

inline std::string_view to_string(Terminal t) {
    return t.is_kw ? to_string(t.kw) : to_string(t.tok);
}
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
     *      k -> Keyword,
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

