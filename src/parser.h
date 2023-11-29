/**
 * @file parser.h
 * @brief Definitions for parser
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 23/11/2023
 */
#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdbool.h>
#include "symtable.h"
#include "symstack.h"

/// Represents a parser state.
typedef struct {
    Token token;      ///< Current non-whitespace token.
    Token token_ws;   ///< Current token before the `g_token`. This can be a Whitespace.
} Parser;

/**
 * @brief Global parser state acessed by parsing functions.
 * @note Defined in `parser.c`, initialized in `parser_init()`.
 * @pre Do not edit this state manually. Use `parser_` functions.
 */
extern Parser g_parser;

/**
 * @brief Global symbol table stack to be used when parsing.
 * @note Defined in `parser.c`, initialized in `parser_init()` and free'd in `parser_free()`.
 */
extern SymStack g_symstack;

/// Initialize the token parser.
void parser_init();

/// Parse the source file passed to `scanner_init()`.
bool parser_begin();

/// Free all resources allocated with parser.
void parser_free();

/**
 * @brief Advance scanner to another token and change Parser state accordingly.
 * @return Pointer to `Parser::token`
 */
Token* parser_next_token();

/**
 * @brief Check if the currently parsed token Parser::token is function ID.
 * @note This will search the SymStack.
 * @return `True` if current token IS function identifier, `false` otherwise.
 */
bool parser_tok_is_fun_id();

#endif // _PARSER_H_

