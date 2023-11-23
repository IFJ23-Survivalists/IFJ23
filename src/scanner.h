/**
 * @file scanner.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 25/09/2023
 * @brief Header file for the lexical analyzer (scanner) module.
 *
 * This module provides functions and data structures for performing lexical analysis on a source file,
 * producing tokens that represent the recognized language constructs.
 */

#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <stdbool.h>
#include "string.h"

/// Represents built-in keywords (but not data type)
typedef enum {
    Keyword_If,
    Keyword_Else,
    Keyword_Let,
    Keyword_Var,
    Keyword_While,
    Keyword_Func,
    Keyword_Return,
} Keyword;

/// Represents data type
typedef enum {
    DataType_Int,
    DataType_Double,
    DataType_String,
    DataType_Nil,
    DataType_MaybeInt,
    DataType_MaybeDouble,
    DataType_MaybeString,
} DataType;

typedef union {
    String string;
    int number;
    double number_double;
} DataValue;

typedef struct {
    DataType type;
    DataValue value;
} Data;

typedef enum {
    /// +
    Operator_Plus,
    /// -
    Operator_Minus,
    /// *
    Operator_Multiply,
    /// /
    Operator_Divide,
    /// ==
    Operator_DoubleEqual,
    /// !=
    Operator_NotEqual,
    /// <
    Operator_LessThan,
    /// <=
    Operator_LessOrEqual,
    /// >
    Operator_MoreThan,
    /// >=
    Operator_MoreOrEqual,
    /// ??
    Operator_DoubleQuestionMark
} Operator;

typedef enum {
    /// EOF
    Token_EOF,

    /// Comment or whitespace character
    Token_Whitespace,

    /// {
    Token_BracketLeft,
    /// }
    Token_BracketRight,
    /// (
    Token_ParenLeft,
    /// )
    Token_ParenRight,
    /// :
    Token_DoubleColon,
    /// ->
    Token_ArrowRight,
    /// =
    Token_Equal,
    /// ,
    Token_Comma,

    /// Data value, can be a string or number or double
    Token_Data,
    /// String, Int, Double
    Token_DataType,
    /// Operator such as + - * / == > >= < <= ??
    Token_Operator,
    /// Language defined keyword
    Token_Keyword,
    /// User defined identifier
    Token_Identifier
} TokenType;

typedef union {
    Keyword keyword;
    DataType data_type;
    Operator op;
    Data data;
    bool has_eol;
} TokenAttribute;

typedef struct {
    TokenType type;
    TokenAttribute attribute;
    size_t line;
    size_t position_in_line;
} Token;

/**
 * @brief Initialize a Scanner instance with a source file.
 *
 * This function initializes the scanner with the provided source file. It prepares the scanner
 * to start analyzing the source file.
 *
 * @param[in] src Pointer to the source file to be analyzed.
 */
void scanner_init(FILE *src);

/**
 * @brief Clean up resources associated with a Scanner instance.
 *
 * This function releases any resources allocated for the scanner, such as closing the source file.
 */
void scanner_free();

/**
 * @brief Reset the scanner to its initial position in the input source.
 *
 * This function resets the scanner to its initial position in the input source, allowing
 * subsequent scans to start from the beginning. It is useful when you need to re-scan
 * the input or when the scanner has reached the end of the source and you want to
 * start a new parsing session.
 */
void scanner_reset_to_beginning();

/**
 * @brief Advance the scanner to recognize the next token.
 *
 * This function advances the scanner's state to recognize the next token in the source file.
 * It returns the recognized token and updates the scanner's internal state accordingly.
 *
 * @note If the attribute is of type String (Identifier or Data) and you intend to use it as your
 *       own string, it's important to clone it using the `string_clone` function. You are then
 *       responsible for deallocating this cloned string when you're done with it.
 *       If one or more errors occur during tokenization, the LexicalError or InternalError will be set.
 *
 * @return The recognized Token.
 */
Token scanner_advance();

/**
 * @brief Advance the scanner to recognize the next non-whitespace token.
 *
 * The same as `scanner_advance` but all whitespace are ignored
 *
 * @note If one or more errors occur during tokenization, the LexicalError or InternalError will be set.
 *
 * @return The recognized Token.
 */
Token scanner_advance_non_whitespace();

#endif
