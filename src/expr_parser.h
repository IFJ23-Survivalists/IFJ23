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
    ///  pushdown is reduced to only on non-terminal) */
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
    DataType type; /**< resulted type after applying a reduction rule */
    Frame frame;
    char* code_name;
    char* param_name;
    bool is_nil;
    char name;     /**< E or L */
    bool is_const; /**< `true` only if const reduced to nonterminal, otherwise `false`*/
} NTerm;

// typedef struct {
//     void* ptr;
//     Pool* next;
// } Pool;

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
 * @param[in] operands Array of operands needed for applying rule.
 * @param[in,out] nterm Nonterminal that holds information obtain from the application of the rule.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_identifier(struct PushdownItem** operands, NTerm* nterm);

/**
 * @brief Propagete data from nonterminal operand to `nterm`.
 * @param[in] operands Array of operands needed for applying rule.
 * @param[in,out] nterm Nonterminal that holds information obtain from the application of the rule.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_parenthesis(struct PushdownItem** operands, NTerm* nterm);

/**
 * @brief Apply rule for reducing logic and arithmetic negation. Checks type of the operands(prints error message if
 * there is a operand mismatch).
 * @param[in] operands Array of operands needed for applying rule.
 * @param[in,out] nterm Nonterminal that holds information obtain from the application of the rule.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_prefix(struct PushdownItem** operands, NTerm* nterm);

/**
 * @brief Apply rule for unwrapping nullable type. For instance convert data type Int? to Int. If operand was null
 * runtime error will occure.
 * @param[in] operands Array of operands needed for applying rule.
 * @param[in,out] nterm Nonterminal that holds information obtain from the application of the rule.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_postfix(struct PushdownItem** operands, NTerm* nterm);

/**
 * @brief Apply rule for arithmetic operations (+-*\/). Checks type of the operands and if possible converts operand to
 * have the same data type (prints error message if there is a operand mismatch).
 * @param[in] operands Array of operands needed for applying rule.
 * @param[in,out] nterm Nonterminal that holds information obtain from the application of the rule.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_arithmetic(struct PushdownItem** operands, NTerm* nterm);

/**
 * @brief Apply rule for logic operations (== < > != <= >=). Checks type of the operands and if possible converts
 * operand to have the same data type (prints error message if there is a operand mismatch).
 * @param[in] operands Array of operands needed for applying rule.
 * @param[in,out] nterm Nonterminal that holds information obtain from the application of the rule.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_logic(struct PushdownItem** operands, NTerm* nterm);

/**
 * @brief Apply rule for replacing left operand if nil by a default value from the right operand. Checks type of the
 * operands(prints error message if there is a operand mismatch). If left operand is not nullable, right operand is
 * ignored.
 * @param[in] operands Array of operands needed for applying rule.
 * @param[in,out] nterm Nonterminal that holds information obtain from the application of the rule.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_nil_coalescing(struct PushdownItem** operands, NTerm* nterm);

/**
 * @brief Apply rule for processing unnamed arguments as well as checking number of parameters. Prints error message if
 * there is too many argumets provided to function or unnamed has type mismatch.
 * @param[in] operands Array of operands needed for applying rule.
 * @param[in,out] nterm Nonterminal that holds information obtain from the application of the rule.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_args(struct PushdownItem** operands, NTerm* nterm);

/**
 * @brief Apply rule for processing named arguments as well as checking number of parameters. Prints error message if
 * there is an type mismatch.
 * @param[in] operands Array of operands needed for applying rule.
 * @param[in,out] nterm Nonterminal that holds information obtain from the application of the rule.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_named_arg(struct PushdownItem** operands, NTerm* nterm);

/**
 * @brief Apply rule for reducing function expression. Checks if there is enough arguments provided otherwise prints
 * error message. Process remaining argument if not processed yet.
 * @param[in] operands Array of operands needed for applying rule.
 * @param[in] id Name of the function.
 * @param[in,out] nterm Nonterminal that holds information obtain from the application of the rule.
 * @return Non terminal that holds data obtained by reducing `operands`.
 */
NTerm* reduce_function(NTerm* nterm, Token* id, NTerm* arg);

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

// /**
//  * @brief Checks if there is not too many arguments provided to a given function.
//  * @param[in] fn_node Contains data about function.
//  * @return `true` if there is not too many arguments provided to a given function, otherwise returns `false`.
//  */
// bool is_valid_num_of_param(StackNode* fn_node);

// /**
//  * @brief Checks if there is an unnamed argument at a given position in function as well as its data type.
//  * @param[in] fn_node Contains data about function.
//  * @param[in] arg Argument value for checking.
//  * @return `true` if argument type and position are correct, otherwise returns `false`.
//  */
// bool process_unnamed_arg(StackNode* fn_node, NTerm* arg);

// /**
//  * @brief Checks if there is a named argument at a given position in function as well as its data type.
//  * @param[in] fn_node Contains data about function.
//  * @param[in] arg_name The external name of argument.
//  * @param[in] arg_value Argument value for checking.
//  * @return `true` if argument type, name and position are correct, otherwise returns `false`.
//  */
// bool process_named_arg(StackNode* fn_node, char* arg_name, NTerm* arg_value);

#endif  // _EXPR_PARSER_H_
