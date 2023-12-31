/// @note Project: Implementace překladače imperativního jazyka IFJ23
/// @file error.c
/// @author Le Duy Nguyen, xnguye27, VUT FIT
/// @author Jakub Kloub, xkloub03, VUT FIT
/// @date 25/09/2023
/// @brief Implementation of functions defined in `error.h` file

#include "error.h"
#include <stdarg.h>
#include "scanner.h"

const char* MSG[] = {
    [1] = "Chyba v programu v rámci lexikální analýzy (chybná struktura aktuálního lexému)",
    "Chyba v programu v rámci syntaktické analýzy (chybná syntaxe programu, chybějící hlavička, atp.)",
    "Sémantická chyba v programu – nedefinovaná funkce",
    "Sémantická chyba v programu – špatný počet/typ parametrů u volání funkce či špatný typ návratové hodnoty z funkce",
    "sémantická chyba v programu – použití nedefinované proměnné.",
    "sémantická chyba v programu – chybějící/přebývající výraz v příkazu návratu z funkce.",
    "sémantická chyba typové kompatibility v aritmetických, řetězcových a relačních výrazech.",
    "sémantická chyba odvození typu – typ proměnné nebo parametru není uveden a nelze odvodit od použitého výrazu.",
    "ostatní sémantické chyby.",
    [99] = "interní chyba překladače tj. neovlivněná vstupním programem (např. chyba alokace paměti atd.)."};

Error ERROR = Error_None;

bool g_print_errors = true;

void set_error(Error err) {
    ERROR = err;
}

Error got_error() {
    return ERROR;
}

void print_error_msg() {
    if (!g_print_errors)
        return;
    if (ERROR) {
        fprintf(stderr, "ERROR: %s\n", MSG[ERROR]);
    }
}

void set_print_errors(bool b) {
    g_print_errors = b;
}

void print_error(const struct Token* tok, Error err_type, const char* err_string, const char* fmt, ...) {
    if (g_print_errors) {
        fprintf(stderr, BOLD("line:%lu:%lu ") COL_R("%s error") ": ", tok->line, tok->position_in_line, err_string);
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
        fprintf(stderr, "\n");
    }

    set_error(err_type);
}

const char* INT_ERR_MSG[] = {[0] = "None",
                             "Invalid argument error",
                             "Memory allocation error",
                             "Out-of-range error",
                             "Runtime error"};

IntError g_int_error = {.type = IntError_None};

void set_int_error(IntErrorType type, const char* msg, const char* file, unsigned int line) {
    g_int_error.type = type;
    g_int_error.msg = msg;
    g_int_error.file = file;
    g_int_error.line = line;
    set_error(Error_Internal);
}

IntErrorType got_int_error() {
    return g_int_error.type;
}

void print_int_error_msg() {
    if (!g_print_errors)
        return;
    if (g_int_error.type) {
#ifdef PRINT_INT_ERR
        if (g_int_error.msg)
            fprintf(stderr, "<internal> " BOLD("%s:%u") " " COL_R("%s") ": %s\n", g_int_error.file, g_int_error.line,
                    INT_ERR_MSG[(int)g_int_error.type], g_int_error.msg);
        else
            fprintf(stderr, "<internal> " BOLD("%s:%u") " " COL_R("%s") "\n", g_int_error.file, g_int_error.line,
                    INT_ERR_MSG[(int)g_int_error.type]);
#endif
    }
}

void clear_int_error() {
    g_int_error.type = IntError_None;
    ERROR = Error_None;
}
