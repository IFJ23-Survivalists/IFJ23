/**
 * @brief Implementation for the parser.h
 * @file parser.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#include "parser.h"
#include "rec_parser.h"
#include "scanner.h"

Parser g_parser;

void parser_init() {
    symstack_init();
    if (got_error())
        return;

    // Create the symbol table used for global variables.
    symstack_push();
}

bool parser_begin() {
    if (rec_parser_begin(ParserMode_Collect)) {
        scanner_reset_to_beginning();
        return rec_parser_begin(ParserMode_Parse);
    }
    return false;
}

void parser_free() {
    symstack_free();
}

Token* parser_next_token() {
    g_parser.token_ws = scanner_advance();
    g_parser.token = g_parser.token_ws.type == Token_Whitespace
                   ? scanner_advance()
                   : g_parser.token_ws;
    return &g_parser.token;
}

bool parser_tok_is_fun_id() {
    MASSERT(false, "Not yet implemented.");
    return g_parser.token.type == Token_Identifier
        && *symtable_get_symbol_type(symstack_top(), g_parser.token.attribute.data.value.string.data) == NodeType_Function;
}

