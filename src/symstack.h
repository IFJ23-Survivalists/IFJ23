/**
 * @brief Symbol table stack declarations
 * @file symstack.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 25/11/2023
 */
#ifndef _SYMSTACK_H_
#define _SYMSTACK_H_

#include "symtable.h"

/// Single node in he symbol table stack linked list.
typedef struct SymStackNode {
    Symtable symtable;          ///< Symbol table this stack node holds.
    struct SymStackNode* next;  ///< Pointer to the next node. NULL means this is the last node.
} SymStackNode;

/**
 * @brief Represents the symbol table stack.
 *
 * Uses single linked list for storing the symtables.
 */
typedef struct {
    size_t size;            ///< Number of node in the stack.
    SymStackNode* top;    ///< Pointer to the top node.
} SymStack;

/**
 * @brief Initialize symbol table stack to empty state.
 * @param[in,out] ss Symbol table stack to initialize
 * @return True on success, False on error.
 * @pre Will also set Error_Internal on error.
 */
bool symstack_init(SymStack* ss);
/**
 * @brief Free all resources of symbol table stack.
 * @param[in,out] ss Symstack to free resources of.
 * @note Calling `symstack_` functions on free'd symstack is undefined.
 * @pre Will set Error_Internal on error.
 */
void symstack_free(SymStack* ss);
/**
 * @brief Pop and free all symbol tables on the stack.
 * @param[in,out] ss Symbol table stack to clear.
 * @pre Will set Error_Internal on error.
 */
void symstack_clear(SymStack* ss);
/**
 * @brief Check if the symbol table stack is empty
 * @param[in] ss Symbol table stack to check.
 * @return `True` if empty, `false` if not.
 * @pre Will set Error_Internal on error.
 */
bool symstack_empty(const SymStack* ss);
/**
 * @brief Get the first symtable from the symtable stack.
 *
 * For example when we are in definition of function this will return the first
 * symtable that is on the symtable stack.
 * @param[in] ss Symbol table stack to create symtable in.
 * @pre Will set Error_Internal on error.
 */
Symtable* symstack_top(const SymStack* ss);
/**
 * @brief Create new Symbol table at the top of the stack.
 * @param[in,out] ss Symbol table stack to create symtable in.
 * @return Pointer to newly created initialized symbol table on success, otherwise NULL.
 * @pre Will set Error_Internal on error.
 */
Symtable* symstack_push(SymStack* ss);
/**
 * @brief Destroy and pop symbol table on the top of the stack
 * @param[in,out] ss Symbol table stack to create symtable in.
 * @return `True` on success, `False` on failure.
 * @pre Will set Error_Internal on error.
 */
bool symstack_pop(SymStack* ss);
/**
 * @brief Search the stack for symbol.
 *
 * Search from top to bottom of the stack for the first symbol table that contains
 * symbol with given name.
 * @param[in] ss Symbol table stack to create symtable in.
 * @param[in] sym_name Symbol to search for.
 * @return `Pointer to symbol table` containing symbol or `NULL` if the symbol doesn't exist.
 * @pre Will set Error_Internal on error.
 */
Symtable* symstack_search(const SymStack* ss, const char* sym_name);

#endif  // _SYMSTACK_H_
