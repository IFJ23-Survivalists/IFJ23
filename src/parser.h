/**
 * @file parser.h
 * @brief Definitions for parser
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 23/11/2023
 */
#ifndef _PARSER_H_
#define _PARSER_H_

#include <signal.h>
#include <stdbool.h>
#include "symtable.h"
#include "symstack.h"
#include "codegen.h"

typedef enum {
    Scope_Local,    ///< Scope inside function
    Scope_Global,   ///< Scope outside function
} Scope;

/// Represents a parser state.
typedef struct {
    Token token;      ///< Current non-whitespace token.
    Token token_ws;   ///< Current token before the `g_token`. This can be a Whitespace.
    Scope scope;      ///< Currenct scope of the parser.
    /// Code of all global statements.
    CodeBuf global_code;
    /// Pointer to currect buffer to which to generate.
    CodeBuf* currect_code;
    /// Used for counting number of declarations in global scope. This
    /// is used for unique name for each variable in global scope.
    int global_var_counter;
    /// Used for counting number of declarations in local scope. This is
    /// used for unique names in functions.
    /// @note This variable is reset each time we enter a function, but the uniquenes remains, because we also add function name to it.
    int local_var_counter;
} Parser;

/**
 * @brief Global parser state acessed by parsing functions.
 * @note Defined in `parser.c`, initialized in `parser_init()`.
 * @pre Do not edit this state manually. Use `parser_` functions.
 */
extern Parser g_parser;

/// Initialize the token parser.
void parser_init();

/**
 * @brief Parse the source file passed to `scanner_init()`.
 * @param output_code If set to TRUE, then IFJcode23 will be outputed to stdout.
 * @return TRUE on parsing success, FALSE otherwise.
 * @pre Error states are set on errors.
 */
bool parser_begin(bool output_code);

/// Free all resources allocated with parser.
void parser_free();

/**
 * @brief Set current scope to given function.
 *
 * When we enter a function during parsing, we need to change the scope,
 * so that we know which frames to use during code generation. Also this
 * will set which buffer we are generating the code into.
 * @param func Function which to set as current scope, or NULL for global scope.
 */
void parser_scope_function(FunctionSymbol* func);

/// Set the scope to global. Equivalent to calling `scope_function(NULL)`.
void parser_scope_global();

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

/**
 * @brief Create string in format <name>%<index> and place it inside str..
 *
 * @param[out] str Initialized string which will contain the output.
 * @param[in] name Name of the variable.
 * @param[in] index Index of the variable.
 */
void create_var_name(String* str, const char* name, int index);

/**
 * @brief Create code name for each parameter in function.
 *
 * This function will create code name for each parameter. This name will be in
 * the format 'TF@<param::iname>%<param_index>'. When the function is called we will
 * use these parameters to push to temporary frame.
 * @param[in,out] func Function to generate the names for.
 */
void parser_parameter_code_infos(FunctionSymbol* func);

/**
 * @brief Create code name for given variable.
 *
 * This function creates a unique code name in given scope for given variable. It takes into account the
 * global parser scope, which is set by functions function_scope_function() and funciton_scope_global().
 * @param[in,out] var Variable symbol to store the code name in.
 * @param[in] name Name of the variable. This name must be the same as in symbol table, in which this variable will be.
 */
void parser_variable_code_info(VariableSymbol* var, const char* name);

/**
 * @brief Create code name (label name) for the given function.
 *
 * @param[in,out] func Function to HJ
 * @param[in] name Name of the function. This needs to be same as when you insert the function into symtable.
 */
void parser_function_code_info(FunctionSymbol* func, const char* name);

#endif // _PARSER_H_
