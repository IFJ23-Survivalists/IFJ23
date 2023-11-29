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
    "Int", "Double", "String", "Bool",
    "Nil",
    "Int?", "Double?", "String?", "Bool?"
};

/**
 * @brief Convert ::DataType to string.
 * @param[in] dt DataType to convert
 * @return Const reference to string literal.
 */
inline static const char* datatype_to_string(DataType dt) { return DATATYPE_NAMES[dt]; }

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
inline static const char* operator_to_string(Operator op) { return OPERATOR_NAMES[op]; }

/// String names for ::TokenType enum.
static const char* TOKENTYPE_NAMES[] = {
    "EOF", "Whitespace",
    "{", "}", "(", ")", ":", "->", "=", ",",
    "if", "else", "let", "var", "while", "func", "return",
    "Data", "DataType",
    "Operator", "Identifier"
};

/**
 * @brief Convert ::TokenType to string.
 * @param[in] tt TokenType to convert
 * @return Const reference to string literal.
 */
inline static const char* tokentype_to_string(TokenType tt) { return TOKENTYPE_NAMES[tt]; }

static const char* token_to_string(const Token* tok) {
    switch (tok->type) {
        case Token_Whitespace:
            if (tok->attribute.has_eol)
                return "EOL";
            return TOKENTYPE_NAMES[tok->type];
        case Token_Data:
            switch (tok->attribute.data.type) {
                case DataType_Int:
                case DataType_Double:
                case DataType_MaybeInt:
                case DataType_MaybeDouble:
                    return "Numeric constant";
                case DataType_Nil:
                    return "Nil";
                case DataType_String:
                case DataType_MaybeString:
                    return "String literal";
                case DataType_Bool:
                case DataType_MaybeBool:
                    return "Boolean value";
            }
            return "Unknown";
        case Token_DataType:
            return datatype_to_string(tok->attribute.data_type);
        case Token_Operator:
            return operator_to_string(tok->attribute.op);
        case Token_Identifier:
            return tok->attribute.data.value.string.data;
        default:
            return tokentype_to_string(tok->type);
    }
}

#endif // _TO_STRING_H_
