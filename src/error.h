/**
 * @brief Define the `Error` enum type and its related function to be used for error handling in the project
 *
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 08/10/2023
 * @file error.h
 */

#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

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
#endif
