/**
 * @brief Declarations for precedence analysis
 * @file expr_parser.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @author Matúš Moravčík, xmorav48, FIT VUT
 * @date 23/11/2023
 */
#ifndef _EXPR_PARSER_H_
#define _EXPR_PARSER_H_

#include <stdbool.h>
#include "function_stack.h"
#include "scanner.h"
#include "symtable.h"

#define RULE_COUNT 14  // Number of valid rules (`Rule::No_Rule` is not counted)

/**
 * @enum ComprarisonResult
 * @brief Represents the result of the precedence comparison of the pushdown terminal and the input token.
 */
typedef enum {
    Left,  /**< Higher precedence of topmost token in Pushdown. */
    Right, /**< Lower precedence of topmost token in Pushdown. */
    Equal, /**< Equal precedence. */

    ///  No relation between tokens => end of operator precedence analysis, potentially error state (depends whether
    ///  pushdown is reduced to only on non-terminal)
    Err,
} ComprarisonResult;

/**
 * @enum PrecedenceCat
 * @brief Represents the result of comparison of Pushdown terminal and input token.
 */
typedef enum {
    PrecendeceCat_PlusMinus,     /**< + - */
    PrecendeceCat_MultiDiv,      /**< * \/*/
    PrecendeceCat_Logic,         /**< == != <= >= < > */
    PrecendeceCat_NilCoalescing, /** ?? */
    PrecendeceCat_Pre,           /**< - ! */
    PrecendeceCat_Post,          /**< ! */
    PrecendeceCat_LeftPar,       /**< ( */
    PrecendeceCat_RightPar,      /**< ) */
    PrecendeceCat_Id,            /**< id const */
    PrecendeceCat_Comma,         /**< , */
    PrecendeceCat_Colon,         /**< : */
    PrecendeceCat_Expr_End,      /**< $ */
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
    Rule_NilCoalescing,   /**< E -> E ?? E */
    Rule_ArgsEE,          /**< L -> E, E */
    Rule_ArgsLE,          /**< L -> L, E */
    Rule_FnArgsProcessed, /**< E -> i(L) */
    Rule_FnArgs,          /**< E -> i(E) */
    Rule_FnEmpty,         /**< E -> i() */
    Rule_NamedArg,        /**< E -> i:E */
    NoRule,               /**< No rule corresponding to given operands */
} Rule;

/**
 * @struct NTerm
 * @brief Rules for expression parsing
 */
typedef struct NTerm {
    DataType type;    /**< resulted type after applying a reduction rule */
    Frame frame;      /** Frame where the value is stored */
    char* code_name;  /** Name of the variable on the frame */
    char* param_name; /** Name of the function parameter */
    bool is_nil;      /** Tells whether constant is nil */
    char name;        /**< E or L */
    bool is_const;    /**< `true` only if const reduced to nonterminal, otherwise `false`*/
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
bool expr_parser_begin(Data* data);

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
 * @param[in] cat Precedence category.
 * @return Character representation of a given precedence category.
 */
char precedence_to_char(PrecedenceCat cat);

/**
 * @brief Recursively parse expression until there is `Err` between topmost pushdown terminal and input terminal.
 * @param[in, out] pushdown Initialized pushdown.
 * @param[in] token Token to be processed.
 * @param[in] prev_token Previous token for deciding ambiguous tokens precedence category.
 */
void parse(Token token, Token* prev_token);

/**
 * @brief Decide which rule should be used for a given string
 * @param[in] rule String representation of right side of rule.
 * @return Corresponding `Rule` or NoRule if no match was found.
 */
Rule get_rule(char* rule);

/**
 * @brief Tries find `fn_name` in symtable. If function with a corresponding name is not found in symtable an error is
 * printed and `NULL is returned.
 * @param[in] fn_name Name of function.
 * @return `FunctionSymbol` of function with a `fn_name` if found in symtable, otherwise returns `NULL`.
 */
FunctionSymbol* get_fn_symbol(String fn_name);

/**
 * @brief Remove all terminals and nonterminals from `pushdown` and replace them with nonterminal created by applying a
 * corresponding rule.
 * @return `true` if reduction was performed. otherwise `false`.
 */
bool reduce();

/**
 * @brief Create non terminal by applying rule to `operands`.
 * @param[in] rule Rule that is used to reduce operand to a new non terminal.
 * @param[in] operands operands needed for reduction.
 * @return Non terminal that holds its type and value infered from `operands` by applying `rule`.
 */
NTerm* apply_rule(Rule rule, struct PushdownItem** operands);

/**
 * @brief Checks exitence of identifier in symtable. If identifier was not found prints corresponding error message and
 * return `NULL`. If identifier found return `NTerm` that holds information about identifier or immediate value. When
 * immediate value is reduced `NTerm::is_const` is set to `true`.
 * @param[in] id Token representing variable of constant.
 * @param[in,out] nterm Non-terminal with default attributes set.
 * @return Non terminal that holds data about variable/constant.
 */
NTerm* reduce_identifier(Token* id, NTerm* nterm);

/**
 * @brief Propagete data from nonterminal operand to `nterm`.
 * @param[in] expr The non-terminal to which the rule applies.
 * @param[out] nterm Non-terminal with default attributes set.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_parenthesis(NTerm* expr, NTerm* nterm);

/**
 * @brief Apply rule for reducing logic and arithmetic negation. Checks type of the operands(prints error message if
 * there is a operand mismatch).
 * @param[in] op Prefix operand. Either `-` or `!`.
 * @param[in] expr The non-terminal to which the rule applies.
 * @param[in,out] nterm Non-terminal with default attributes set.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_prefix(Operator op, NTerm* expr, NTerm* nterm);

/**
 * @brief Apply rule for unwrapping nullable type. For instance convert data type Int? to Int. If operand was null
 * runtime error will occure.
 * @param[in] expr The non-terminal to which the rule applies.
 * @param[in,out] nterm Non-terminal with default attributes set.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_postfix(NTerm* expr, NTerm* nterm);

/**
 * @brief Apply rule for arithmetic operations (+-*\/). Checks type of the operands and if possible converts operand to
 * have the same data type (prints error message if there is a operand mismatch).
 * @param[in] left The left operand in binary expression.
 * @param[in] op Binary operator: '+', '-', '*', '/'
 * @param[in] right The right operand in binary expression.
 * @param[in,out] nterm Non-terminal with default attributes set.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_arithmetic(NTerm* left, Operator op, NTerm* right, NTerm* nterm);

/**
 * @brief Apply rule for logic operations (== < > != <= >= && ||). Checks type of the operands and if possible converts
 * operand to have the same data type (prints error message if there is a operand mismatch).
 * @param[in] left The left operand in binary expression.
 * @param[in] op Binary operator: '==', '!=', '<', '>', '&&', '||', '>=', '<='
 * @param[in] right The right operand in binary expression.
 * @param[in,out] nterm Non-terminal with default attributes set.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_logic(NTerm* left, Operator op, NTerm* right, NTerm* nterm);

/**
 * @brief Apply rule for replacing left operand if nil by a default value from the right operand. Checks type of the
 * operands(prints error message if there is a operand mismatch). If left operand is not nullable, right operand is
 * ignored.
 * @param[in] left The left operand in binary expression.
 * @param[in] right The right operand in binary expression. This operand cannot be nil or nullable data type
 * @param[in,out] nterm Non-terminal with default attributes set.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_nil_coalescing(NTerm* left, NTerm* right, NTerm* nterm);

/**
 * @brief Apply rule for processing named arguments as well as checking number of parameters. Prints error message if
 * there is an type mismatch.
 * @param[in] id Name of the function
 * @param[in] arg Function argument value.
 * @param[in,out] nterm Non-terminal with default attributes set.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_named_arg(Token* id, NTerm* arg, NTerm* nterm);

/**
 * @brief Apply rule for processing unnamed arguments as well as checking number of parameters. Prints error message if
 * there is too many argumets provided to function or unnamed has type mismatch.
 * @param[in] left The first argument of the function ('E') or non-terminal that was created by applying this rule
 * muliple times ('L')
 * @param[in] right The next argument to be inserted into function parameter stack.
 * @param[in,out] nterm Non-terminal with default attributes set.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_args(NTerm* left, NTerm* right, NTerm* nterm);

/**
 * @brief Apply rule for reducing function expression. Checks if there is enough arguments provided and parameter names
 * as well as their data types with comparison to function declaration
 * @param[in] id Name of the function.
 * @param[in] arg Function argument. Set to `NULL` if function does not have any parameters.
 * @param[in,out] nterm Non-terminal with default attributes set.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_function(Token* id, NTerm* arg, NTerm* nterm);

/**
 * @brief Tries convert one operand type to the type of the other one. Conversion is possible only for immediate values
 * (expression that was created by reducing a immediate value)
 * @param[in] op1 Operand for conversion.
 * @param[in] op2 Operand for conversion.
 * @return `true` if conversion was successful or operands are of the same type, otherwise returns `false`.
 */
bool try_convert_to_same_types(NTerm* op1, NTerm* op2);

/**
 * @brief Tries convert operands one operand type to the type of the other one. Conversion is possible only for literals
 * (expression that was created by reducing an immediate value)
 * @param[in] dt `DataType` that `op` is converted to, if possible.
 * @param[in] operand operand for type conversion (only possible for immediate values).
 * @param[in] allow_nil If `true` it allows nil being right operand for nullable `dt`.
 * @return `true` if conversion was successful or `op` is of the same type as `dt`, otherwise returns `false`.
 */
bool try_convert_to_datatype(DataType dt, NTerm* operand, bool allow_nil);

#endif  // _EXPR_PARSER_H_
