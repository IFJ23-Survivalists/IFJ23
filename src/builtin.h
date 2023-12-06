/**
 * @note Project: Implementace překladače imperativního jazyka IFJ23
 * @file builtin.h
 * @author Lukáš Habr
 * @brief Add builtin functions.
 * @date 5/12/2023
 */
#ifndef _BUILTIN_H_
#define _BUILTIN_H_

/// Creates builtin readString function.
void builtin_add_readString();
/// Creates builtin readInt function.
void builtin_add_readInt();
/// Creates builtin readDouble function.
void builtin_add_readDouble();
/// Creates builtin readBool function.
void builtin_add_readBool();
/// Creates builtin write function.
void builtin_add_write();
/// Creates builtin Int2Double function.
void builtin_add_Int2Double();
/// Creates builtin Double2Int function.
void builtin_add_Double2Int();
/// Creates builtin length function.
void builtin_add_length();
/// Creates builtin substring function.
void builtin_add_substring();
/// Creates builtin ord function.
void builtin_add_ord();
/// Creates builtin chr function.
void builtin_add_chr();

#endif
