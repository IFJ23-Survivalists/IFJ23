/**
 * @brief Define the `Error` enum type and its related function to be used for error handling in the project
 *
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 08/10/2023
 * @file error.h
 */

#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include "color.h"

#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#else

/// Print the debug string into `stderr`
#define debug(s) fprintf(stderr, __FILE__":%u: %s\n",__LINE__, s)

/// Print the debug string with format like printf
#define dfmt(s, ...) fprintf(stderr, __FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

#endif

/// Print the error string into `stderr`
#define eprint(s) fprintf(stderr, s)

/// Print the error string with format like printf
#define eprintf(s, ...) fprintf(stderr, s, __VA_ARGS__)

/// Color used when printing error keyword.
#define ERR_COL COL_R


/// Represents various error types that can occur in the application.
typedef enum {
    /// No error. Yay!
    Error_None,
    /// chyba v programu v rámci lexikální analýzy (chybná struktura aktuálního lexému).
    Error_Lexical,

    /// chyba v programu v rámci syntaktické analýzy (chybná syntaxe programu, chybějící hlavička, atp.).
    Error_Syntax,

    /// sémantická chyba v programu – nedefinovaná funkce.
    Error_UndefinedFunction,

    /// sémantická chyba v programu – špatný počet/typ parametrů u volání funkce či špatný typ návratové hodnoty z funkce.
    Error_TypeMismatched,

    /// sémantická chyba v programu – použití nedefinované proměnné.
    Error_UndefinedVariable,

    /// sémantická chyba v programu – chybějící/přebývající výraz v příkazu návratu z funkce.
    Error_ReturnValueMismatched,

    /// sémantická chyba typové kompatibility v aritmetických, řetězcových a relačních výrazech.
    Error_Operation,

    /// sémantická chyba odvození typu – typ proměnné nebo parametru není uveden a nelze odvodit od použitého výrazu.
    Error_UnknownType,

    /// ostatní sémantické chyby.
    Error_Semantic,

    /// interní chyba překladače tj. neovlivněná vstupním programem (např. chyba alokace paměti atd.).
    Error_Internal = 99,
} Error;

/**
 * @brief Set global error state.
 */
void set_error(Error);

/**
 * @brief Get the global error state.
 *
 * This function will return the error that was last set with set_error() function.
 * @return Global error state.
 */
Error got_error();

/// @brief Print the `error` message to `stderr`
void print_error_msg();

// Forward declaration.
struct Token;

/**
 * @brief Print given error message based on tok properties.
 * @param[in] tok Token where the error happened. This is used for line and position in line.
 * @param[in] err_type Type of error to set after printing.
 * @param[in] fmt Format in which to print
 * @param ... Argument to fmt
 */
void print_error(const struct Token* tok, Error err_type, const char* err_string, const char* fmt, ...);

#define lex_err(...) print_error(&g_parser.token, Error_Lexical, "Lexical", __VA_ARGS__)
#define syntax_err(...) print_error(&g_parser.token, Error_Syntax, "Syntax", __VA_ARGS__)
#define undef_fun_err(...) print_error(&g_parser.token, Error_UndefinedFunction, "Undefined function", __VA_ARGS__)
#define fun_type_err(...) print_error(&g_parser.token, Error_TypeMismatched, "Type mismatch", __VA_ARGS__)
#define undef_var_err(...) print_error(&g_parser.token, Error_UndefinedVariable, "Undefined variable", __VA_ARGS__)
#define return_err(...) print_error(&g_parser.token, Error_ReturnValueMismatched, "Return value missmatch", __VA_ARGS__)
#define expr_type_err(...) print_error(&g_parser.token, Error_Operation, "Type mismatch", __VA_ARGS__)
#define unknown_type_err(...) print_error(&g_parser.token, Error_UnknownType, "Unknown type", __VA_ARGS__)
#define semantic_err(...) print_error(&g_parser.token, Error_Semantic, "Semantic", __VA_ARGS__)

/// Represents type of internal error.
typedef enum {
    /// No interal error.
    IntError_None = 0,

    /// Invalid arguments passed to a function.
    IntError_InvalidArgument,

    /// Memory allocation errors.
    IntError_Memory,

    /// Out-of-range errors.
    IntError_Range,

    /// Other runtime errors.
    IntError_Runtime,
} IntErrorType;

/// Represents an interal error data.
typedef struct {
    IntErrorType type;
    const char* msg;
    const char* file;
    unsigned int line;
} IntError;

/**
 * @brief Set internal error state.
 *
 * @param type Type of the internal error.
 * @param msg Static message to store. DO NOT PUT A DYNAMIC MSG HERE (the msg needs to be valid when printing).
 * @param file Static string representing the file where an error happened.
 * @param line Line in the file where the error happened.
 * @note This will also set error state to `Error_Internal`
 */
void set_int_error(IntErrorType type, const char *msg, const char *file, unsigned int line);

/// Automatically add file and line when setting internal error.
#define SET_INT_ERROR(type, msg) set_int_error(type, msg, __FILE__, __LINE__)

/// Get the global internal error state.
IntErrorType got_int_error();

/// Print the internal error message.
void print_int_error_msg();

/**
 * @brief Clear the internal error state
 * @note This will also reset the global error state to `Error_None`.
 */
void clear_int_error();

/// Assert with custom message printed to stderr.
#define MASSERT(expr, msg) do {                                                                     \
    if (!(expr)) {                                                                                  \
        fprintf(stderr, __FILE__":%i: Assertion `" #expr "` failed. Message: %s\n", __LINE__, msg); \
        exit(1);                                                                                    \
    }} while(false)

#endif
