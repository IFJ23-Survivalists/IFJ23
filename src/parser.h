/**
 * @file parser.h
 * @brief Definitions for parser
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 23/11/2023
 *
 */
#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdbool.h>
#include "symtable.h"
#include "symstack.h"

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

#endif // _PARSER_H_

