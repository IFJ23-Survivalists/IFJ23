/**
 * @note Project: Implementace překladače imperativního jazyka IFJ23
 * @brief Recursive parser declarations.
 * @file rec_parser.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 23/11/2023
 */
#ifndef _PRED_PARSER_H
#define _PRED_PARSER_H

#include <stdbool.h>
#include "parser.h"

/// Check if g_parser.token.type is `tok` and if not, set syntax_err with given msg format.
#define CHECK_TOKEN(tok, ...)         \
    if (g_parser.token.type != tok) { \
        syntax_err(__VA_ARGS__);      \
        return false;                 \
    }                                 \
    parser_next_token()

/// Call given rule function and return false if the rule fails.
#define CALL_RULE(rule) \
    if (!rule())        \
    return false
/// Call given rule function with parameters and return false if the rule fails.
#define CALL_RULEp(rule, ...) \
    if (!rule(__VA_ARGS__))   \
    return false

/// Same as ::CHECK_TOKEN, but to be used inside TRY blocks
#define bCHECK_TOKEN(tok, ...)        \
    if (g_parser.token.type != tok) { \
        syntax_err(__VA_ARGS__);      \
        _err = 1;                     \
        break;                        \
    }                                 \
    parser_next_token()

/// Same as ::CALL_RULEp but to be used inside TRY blocks.
#define bCALL_RULEp(rule, ...) \
    if (!rule(__VA_ARGS__)) {  \
        _err = 1;              \
        break;                 \
    }
/// Beginning of TRY block
#define TRY_BEGIN     \
    do {              \
        int _err = 0; \
        do
/// Beginning of final block
#define TRY_FINAL \
    while (0)     \
        ;         \
    if (_err)
/// Mark end of TRY block
#define TRY_END \
    }           \
    while (0)
/// Go from inside the TRY block into the FINAL block
#define BREAK_FINAL \
    {               \
        _err = 1;   \
        break;      \
    }
/// Go from inside the TRY block after the TRY_END.
#define BREAK_END \
    {             \
        _err = 0; \
        break;    \
    }

/// Shorthand for getting the string representation of token.
#define TOK_STR token_to_string(&g_parser.token)
/**
 * @brief Begin the predictive analysis.
 * @return `True` if the parsing was successful, `False` on syntax or semantic error in file.
 */
bool rec_parser_begin();

/**
 * @brief Collect function definitions into the symbol table at the top of the SymStack.
 * @return True if successful, false otherwise.
 */
bool rec_parser_collect();

#endif
