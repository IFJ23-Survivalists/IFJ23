/**
 * @note Project: Implementace překladače imperativního jazyka IFJ23
 * @brief Color printing utilities.
 * @file color.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 29/11/2023
 */
#ifndef _COLOR_H_
#define _COLOR_H_

/// Default color
#define PRINT_RESET "\033[0m"
/// Red color
#define PRINT_R "\033[31m"
/// Green color
#define PRINT_G "\033[32m"
/// Yellow color
#define PRINT_Y "\033[33m"
/// Blue color
#define PRINT_B "\033[34m"
/// Magenta color
#define PRINT_M "\033[35m"
/// Cyan color
#define PRINT_C "\033[36m"
/// White color
#define PRINT_W "\033[37m"
/// Print bold text
#define PRINT_BOLD "\033[1m"

/// Convert given string literal to colored string literal
#define COL(color, text) color text COL_D

/// Convert to red text.
#define COL_R(text) PRINT_R text PRINT_RESET
/// Convert to green text.
#define COL_G(text) PRINT_G text PRINT_RESET
/// Convert to yellow text.
#define COL_Y(text) PRINT_Y text PRINT_RESET
/// Convert to blue text.
#define COL_B(text) PRINT_B text PRINT_RESET
/// Convert to magenta text.
#define COL_M(text) PRINT_M text PRINT_RESET
/// Convert to cyan text.
#define COL_C(text) PRINT_C text PRINT_RESET
/// Convert to white text.
#define COL_W(text) PRINT_W text PRINT_RESET
/// Convert to bold text.
#define BOLD(text) PRINT_BOLD text PRINT_RESET

#endif  // _COLOR_H_
