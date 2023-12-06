/// @note Project: Implementace překladače imperativního jazyka IFJ23
/// @file main.c
/// @author Le Duy Nguyen, xnguye27, VUT FIT
/// @author Jakub Kloub, xkloub03, VUT FIT
/// @date 08/10/2023
/// @brief Main program of the IFJ23

#include "error.h"
#include "parser.h"
#include "scanner.h"

int main() {
    // Initialize the parser to use this file for tokens.
    scanner_init(stdin);
    if (got_error()) {
        print_error_msg();
        return 99;
    }

    parser_init();
    if (got_error()) {
        if (got_int_error())
            print_int_error_msg();
        print_error_msg();
        scanner_free();
        return 99;
    }

    // Parse the source file.
    parser_begin(true);
    if (got_error()) {
        if (got_int_error())
            print_int_error_msg();
        print_error_msg();
    }

    parser_free();
    scanner_free();
    return got_error();
}
