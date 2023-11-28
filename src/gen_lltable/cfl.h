/**
 * @file cfl.h
 * @brief Context-free language definitions
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#ifndef _CFL_H_
#define _CFL_H_

#include <stdbool.h>
#include "../scanner.h"

/// Total number of TokenType enum elements.
/// @note Assuming that `TokenType` enum starts from 0 and doesn't skip any numbers.
const int TOK_COUNT = Token_Identifier + 1;

 /// All possible non-terminals
typedef enum {
    NTerm_StatementList = 0,
    NTerm_StatementSeparator,
    NTerm_Statement,
    NTerm_IfStatement,
    NTerm_Params,
    NTerm_Params_n,
    NTerm_ReturnExpr,
    NTerm_IfCondition,
    NTerm_Else,
    NTerm_ElseIf,
    NTerm_AssignType,
    NTerm_AssignExpr,
    NTerm_Expr,

    /// Number of non-terminals in this enum.
    NTerm_count
} NTerm;

/// Represents a terminal in a rule.
typedef struct {
    // TODO: Add terminal attributes or change to Token.

    TokenType tok;  ///< Which TokenType this is
} Terminal;

/// Type to use when the Terminal is translated to number.
/// @note Translation to number can be used for indexing tables and such.
typedef TokenType TermValue;

/**
 * @brief Create terminal from TermValue
 *
 * @param val TermValue representing this terminal. It can be obtained from calling term_to_val() function.
 * @return Newly created terminal.
 */
inline Terminal term_from_val(TermValue val) {
    MASSERT(val >= 0, "Invalid value for convertsion to terminal.");
    Terminal t;
    t.tok = val;
    return t;
}

/**
 * @brief Convert Terminal to TermValue representing this terminal.
 *
 * @param t Terminal to convert
 * @return TermValue representing this terminal.
 */
inline TermValue term_to_val(const Terminal* t) {
    return t->tok;
}

/// Union of Terminals and Non-Terminals
typedef struct {
    bool is_term;   /// Is terminal. To know which union member is used.
    union {
        Terminal term;
        NTerm nterm;
    };

    /// FIXME: This should be moved to src/grammar.h if left ununsed here.
} Symbol;

#endif // _CFL_H_

