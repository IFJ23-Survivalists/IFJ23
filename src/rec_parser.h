/**
 * @brief Recursive parser declarations.
 * @file rec_parser.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 23/12/2023
 */
#ifndef _PRED_PARSER_H
#define _PRED_PARSER_H

#include <stdbool.h>

/// Represents in which mode should the parser run.
typedef enum {
    /// Only collect function definitions into the symbol table.
    ParserMode_Collect,
    /// Parse the entire file again now with semantic actions.
    ParserMode_Parse,
} ParserMode;


/**
 * @brief Begin the predictive analysis.
 * @return `True` if the parsing was successful, `False` on syntax or semantic error in file.
 */
bool rec_parser_begin(ParserMode mode);

#endif
