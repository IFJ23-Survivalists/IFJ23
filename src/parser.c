/**
 * @file parser.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#include "parser.h"
#include "prec_parser.h"


void parser_init() {}

bool parser_begin() {
    return prec_parser_begin();
}

void parser_free() {}
