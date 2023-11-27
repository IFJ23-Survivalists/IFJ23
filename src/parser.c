/**
 * @file parser.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#include "parser.h"
#include "rec_parser.h"

SymStack g_symstack;

void parser_init() {
    symstack_init(&g_symstack);
    if (got_error())
        return;
    // Create the symbol table used for global variables.
    symstack_push(&g_symstack);
}

bool parser_begin() {
    if (rec_parser_begin(ParserMode_Collect)) {
        scanner_reset_to_beginning();
        return rec_parser_begin(ParserMode_Parse);
    }
    return false;
}

void parser_free() {
    symstack_free(&g_symstack);
}
