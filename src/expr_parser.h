/**
 * @brief Declarations for precedence analysis
 * @file prec_praser.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @author Matúš Moravčík, xmorav48, FIT VUT
 * @date 23/11/2023
 */
#ifndef _PREC_PARSER_H_
#define _PREC_PARSER_H_

#include <stdbool.h>
#include "pushdown.h"
#include "scanner.h"

#define RULE_COUNT 13

/**
 * @enum ComprarisonResult
 * @brief Represents the result of the precedence comparison of the pushdown terminal and the input token.
 */
typedef enum {
  Left,  /**< Higher precedence on Pushdown. */
  Right, /**< Current token has higher precedence. */
  Equal, /**< Equal precedence. */
  Err,   /**< No relation between tokens => error state */
} ComprarisonResult;

/**
 * @enum PrecedenceCat
 * @brief Represents the result of comparison of Pushdown terminal and input token.
 */
typedef enum {
  PrecendeceCat_PlusMinus, /**< + - */
  PrecendeceCat_MultiDiv,  /**< * \/*/
  PrecendeceCat_Logic,     /**< == != <= >= < > */
  PrecendeceCat_Pre,       /**< - ! */
  PrecendeceCat_Post,      /**< ! */
  PrecendeceCat_LeftPar,   /**< ( */
  PrecendeceCat_RightPar,  /**< ) */
  PrecendeceCat_Id,        /**< id const */
  PrecendeceCat_Comma,     /**< , */
  PrecendeceCat_Colon,     /**< : */
  PrecendeceCat_Expr_End,  /**< $ */
} PrecedenceCat;

/**
 * @enum Rule
 * @brief Rules for expression parsing
 */
typedef enum {
  Rule_Identif,         /**< E -> i */
  Rule_Paren,           /**< E -> (E) */
  Rule_Prefix,          /**< E -> -E | !E */
  Rule_Postfix,         /**< E -> id! */
  Rule_SumSub,          /**< E -> E + E | E - E */
  Rule_MulDiv,          /**< E -> E * E | E / E */
  Rule_Logic,           /**< E -> E == E | E < E E && E ... */
  Rule_ArgsEE,          /**< L -> E, E */
  Rule_ArgsLE,          /**< L -> L, E */
  Rule_FnArgsProcessed, /**< E -> i(L) */
  Rule_FnArgs,          /**< E -> i(E) */
  Rule_FnEmpty,         /**< E -> i() */
  Rule_NamedArg,        /**< E -> i:E */
  NoRule,
} Rule;

/**
 * @struct NTerm
 * @brief Rules for expression parsing
 */
typedef struct NTerm {
  DataValue value; /**< resulted value after applying a reduction rule */
  DataType type;   /**< resulted type after applying a reduction rule */
  char name;       /**< E or L */
} NTerm;

// Forward declaration
struct Pushdown;
struct PushdownItem;

/**
 * @brief Starts bottom up parsing for expressions
 * @param[in] token Current input token.
 * @param[out] data Data of the resulting reduced nonterminal after applying operator precedence rules.
 * @return `true` if expression was successfully parsed (reduced to just one nonterminal), otherwise `false`.
 */
bool expr_parser_begin(Token token, Data* data);

/**
 * @brief Classify `token` to precedence category. ambiguous token as - or ! are decided be `prev_token`
 * @param[in] token Current input token.
 * @param[in] prev_token Token from the last step.
 * @return `token` precedence category.
 */
PrecedenceCat getTokenPrecedenceCategory(Token token, Token* prev_token);

/**
 * @brief Get precendence from the precedence table
 * @param[in] pushdownItem Pushdown precedence index.
 * @param[in] inputToken Input token precedence index.
 * @return `ComparisonResult`.
 */
ComprarisonResult getPrecedence(PrecedenceCat pushdownItem, PrecedenceCat inputToken);

/**
 * @brief Convert precendence category into its character representation.
 * @param[in] cat Precendence category.
 * @return Character representation of a given precedence category.
 */
char token_to_char(PrecedenceCat cat);

/**
 * @brief Recursively parse expression until there is `Err` between topmost pushdown terminal and input terminal.
 * @param[in, out] pushdown Initialized pushdown.
 * @param[in] token Token to be processed.
 * @param[in] prev_token Previous token for deciding ambiguous tokens precedence category.
 */
void parse(struct Pushdown* pushdown, Token token, Token* prev_token);

/**
 * @brief Decide which rule should be used for a given string
 * @param[in] rule String representation of right side of rule.
 * @return Corresponding `Rule` or NoRule if no match was found.
 */
Rule get_rule(char* rule);

/**
 * @brief Create non terminal by applying rule to `operands`.
 * @param[in] rule Rule that is used to reduce operand to a new non terminal.
 * @param[in] operands operands needed for reduction.
 * @return Non terminal that holds its type and value infered from `operands` by applying `rule`.
 */
NTerm* apply_rule(Rule rule, struct PushdownItem* operands);

/**
 * @brief Remove all terminals and nonterminals from `pushdown` and replace them by non terminal created by applying any
 * rule.
 * @param[in, out] pushdown Initialized pushdown.
 * @return `true` if reduction was performed. otherwise `false`.
 */
bool reduce(struct Pushdown* pushdown);

/**
 * @brief Retrieves the index of the topmost terminal in the Pushdown structure.
 * @param[in] pushdown Pointer to the Pushdown structure.
 * @return The index of the topmost terminal, or -1 if no terminal is found.
 */
int get_topmost_terminal_id(struct Pushdown* pushdown);

#endif  // _PREC_PARSER_H_
