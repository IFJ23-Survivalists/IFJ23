/**
 * @file parser.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#include "parser.h"
#include "rec_parser.h"


Symtable g_symtable;

void parser_init() {}

bool parser_begin() {
    return rec_parser_begin();
}

void parser_free() {}
