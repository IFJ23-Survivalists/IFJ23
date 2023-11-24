/// @file main.c
/// @author Le Duy Nguyen, xnguye27, VUT FIT
/// @author Jakub Kloub, xkloub03, VUT FIT
/// @date 08/10/2023
/// @brief Main program of the IFJ23

#include "error.h"
#include "parser.h"
#include "scanner.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        eprint("The argument count must be exactly 1\n");
        return 99;
    }

    FILE *file = fopen(argv[1], "r+");

    if (!file) {
        eprintf("Unable to read file %s\n", argv[1]);
        return 99;
    }

    // Initialize the parser to use this file for tokens.
    scanner_init(file);
    if (got_error()) {
        print_error_msg();
        fclose(file);
        return 99;
    }

    parser_init();
    if (got_error()) {
        print_error_msg();
        scanner_free();
        fclose(file);
        return 99;
    }

    symtable_init(&g_symtable);
    if (got_int_error()) {
        print_int_error_msg();
        return 99;
    }

    // Parse the source file.
    parser_begin();
    if (got_error())
        print_error_msg();


    symtable_free(&g_symtable);
    parser_free();
    scanner_free();
    fclose(file);
    return got_error();
}
