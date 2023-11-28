/**
 * @brief To string translations various structures.
 *
 * This file contains functions for converting enums and structures to string. This includes ::DataType, ::Operator and ::TokenType.
 * @todo Add translations for structures such as ::Token for better parser error messages.
 * @file to_string.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 28/11/2023
 */
#ifndef _TO_STRING_H_
#define _TO_STRING_H_
#include <stdio.h>
#include <stdarg.h>

#include "scanner.h"

/**
 * @brief Typical `fprintf` that is extended to support some of our enums.
 *
 * Works like `fprintf`, but adds additional format characters. These special format chars start with `#` and are followed by: \n
 *     t ... For ::TokenType \n
 *     d ... For ::DataType \n
 *     o ... For ::Operator \n
 *     \# ... For the regular '#' character
 *
 * @param[out] f File to output to.
 * @param[in] fmt Format in which to print.
 * @param[in] ... Variables to print in respect to `fmt`.
 */
// void sfprintf(FILE* f, const char* fmt, ...);

/// String names for ::DataType enum.
static const char* DATATYPE_NAMES[] = {
    "Int", "Double", "String",
    "Nil",
    "Int?", "Double?", "String?"
};

/**
 * @brief Convert ::DataType to string.
 * @param[in] dt DataType to convert
 * @return Const reference to string literal.
 */
inline const char* datatype_to_string(DataType dt) { return DATATYPE_NAMES[dt]; }

/// String names for ::Operator enum.
static const char* OPERATOR_NAMES[] = {
    "+", "-", "*", "/",
    "==", "!=", "<", "<=", ">", ">=",
    "??", "!", "||", "&&"
};

/**
 * @brief Convert ::Operator to string.
 * @param[in] dt Operator to convert
 * @return Const reference to string literal.
 */
inline const char* operator_to_string(Operator op) { return OPERATOR_NAMES[op]; }

/// String names for ::TokenType enum.
static const char* TOKENTYPE_NAMES[] = {
    "EOF", "Whitespace",
    "{", "}", "(", ")", ":", "->", "=", ",",
    "If", "Else", "Let", "Var", "While", "Func", "Return", "Data", "DataType",
    "Operator", "Identifier",
    "EOL"   // Special case for whitespace with has_eol parameter.
};

/**
 * @brief Convert ::TokenType to string.
 * @param[in] dt TokenType to convert
 * @return Const reference to string literal.
 */
inline const char* tokentype_to_string(TokenType tt) { return TOKENTYPE_NAMES[tt]; }

#endif // _TO_STRING_H_
