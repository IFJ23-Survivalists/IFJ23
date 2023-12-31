/**
 * @note Project: Implementace překladače imperativního jazyka IFJ23
 * @brief To string translations various structures.
 *
 * This file contains functions for converting enums and structures to string. This includes ::DataType, ::Operator and
 * ::TokenType.
 * @file to_string.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 28/11/2023
 */
#ifndef _TO_STRING_H_
#define _TO_STRING_H_
#include <stdarg.h>
#include <stdio.h>

#include "codegen.h"
#include "scanner.h"

/**
 * @brief Typical `fprintf` that is extended to support some of our enums.
 *
 * Works like `fprintf`, but adds additional format characters. These special format chars start with `#` and are
 * followed by: \n t ... For ::TokenType \n d ... For ::DataType \n o ... For ::Operator \n
 *     \# ... For the regular '#' character
 *
 * @param[out] f File to output to.
 * @param[in] fmt Format in which to print.
 * @param[in] ... Variables to print in respect to `fmt`.
 */
// void sfprintf(FILE* f, const char* fmt, ...);

/// String names for ::DataType enum.
static const char* DATATYPE_NAMES[] = {
    "Int", "Double", "String", "Bool", "Int?", "Double?", "String?", "Bool?", "Undefined",
};

/**
 * @brief Convert ::DataType to string.
 * @param[in] dt DataType to convert
 * @return Const reference to string literal.
 */
inline static const char* datatype_to_string(DataType dt) {
    return DATATYPE_NAMES[dt];
}

/// String names for ::Operator enum.
static const char* OPERATOR_NAMES[] = {"+", "-", "*", "/", "==", "!=", "<", "<=", ">", ">=", "??", "!", "||", "&&"};

/**
 * @brief Convert ::Operator to string.
 * @param[in] dt Operator to convert
 * @return Const reference to string literal.
 */
inline static const char* operator_to_string(Operator op) {
    return OPERATOR_NAMES[op];
}

/// String names for ::TokenType enum.
static const char* TOKENTYPE_NAMES[] = {"EOF",   "Whitespace", "{",      "}",    "(",        ")",        ":",
                                        "->",    "=",          ",",      "if",   "else",     "let",      "var",
                                        "while", "func",       "return", "Data", "DataType", "Operator", "Identifier"};

/**
 * @brief Convert ::TokenType to string.
 * @param[in] tt TokenType to convert
 * @return Const reference to string literal.
 */
inline static const char* tokentype_to_string(TokenType tt) {
    return TOKENTYPE_NAMES[tt];
}

/**
 * @brief Get token string representation.
 *
 * @param[in] tok Token to get as a string.
 * @return Static string pointer.
 */
const char* token_to_string(const Token* tok);

/**
 * @brief Convert in to string.
 * @param[in] num Number to convert.
 * @return String with number as string. This string needs to be free'd using string_free().
 */
String unsigned_to_string(unsigned num);

/// String names for ::Frame enum.
static const char* FRAME_NAMES[] = {"GF", "LF", "TF"};

/**
 * @brief Convert frame to string
 * @param op Frame to convert
 * @return Constant string representing frame as string.
 */
inline static const char* frame_to_string(Frame op) {
    return FRAME_NAMES[op];
}

#endif  // _TO_STRING_H_
