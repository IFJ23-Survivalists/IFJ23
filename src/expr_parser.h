/**
 * @brief Declarations for precedence analysis
 * @file prec_praser.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 23/11/2023
 */
#ifndef _PREC_PARSER_H_
#define _PREC_PARSER_H_

#include <stdbool.h>
#include "scanner.h"

/**
 * @brief Begin the precedence analysis
 *
 * @param[in] first_token First token to be processed by the precedence parser
 * @return `True` if the precedence analysis succeded, `false` otherwise.
 */
bool expr_parser_begin(Token first_token);

#endif // _PREC_PARSER_H_
