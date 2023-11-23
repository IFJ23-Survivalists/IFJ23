/**
 * @file parser.h
 * @brief Definitions for parser
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 23/11/2023
 */
#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdbool.h>


/// Initialize the token parser.
void parser_init();
/// Parse the source file passed to `scanner_init()`.
bool parser_begin();
/// Free all resources allocated with parser.
void parser_free();

#endif // _PARSER_H_

