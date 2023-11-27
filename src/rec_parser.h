/**
 * @brief Logic for predictive parser
 * @file rec_parser.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 23/12/2023
 */
#ifndef _PRED_PARSER_H
#define _PRED_PARSER_H

#include <stdbool.h>

/**
 * @brief Begin the predictive analysis.
 * @return `True` if the parsing was successful, `False` on syntax or semantic error in file.
 */
bool rec_parser_begin();

#endif
