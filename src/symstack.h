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
 * @return True on success, False on error.
 * @pre Will also set Error_Internal on error.
 */
bool symstack_init();

/**
 * @brief Free all resources of symbol table stack.
 * @note Calling `symstack_` functions on free'd symstack is undefined.
 * @pre Will set Error_Internal on error.
 */
void symstack_free();

/**
 * @brief Pop and free all symbol tables on the stack.
 * @pre Will set Error_Internal on error.
 */
void symstack_clear();

/**
 * @brief Check if the symbol table stack is empty
 * @return `True` if empty, `false` if not.
 * @pre Will set Error_Internal on error.
 */
bool symstack_empty();

/**
 * @brief Get the current number of symtables on symstack.
 * @return SymStack size
 */
size_t symstack_size();

/**
 * @brief Get the first symtable from the symtable stack.
 *
 * For example when we are in definition of function this will return the first
 * symtable that is on the symtable stack.
 * @pre Will set Error_Internal on error.
 */
Symtable* symstack_top();

/**
 * @brief Get the symbol table at the bottom of the stack.
 *
 * This will return the symbol table at the bottom of the stack. We need this,
 * because at the bottom of the stack there is the GLOBAL symbol table, in which
 * functions are to be defined.
 * @return The symbol table at the bottom of NULL when the symstack is empty.
 */
Symtable* symstack_bottom();

/**
 * @brief Create new Symbol table at the top of the stack.
 * @return Pointer to newly created initialized symbol table on success, otherwise NULL.
 * @pre Will set Error_Internal on error.
 */
Symtable* symstack_push();

/**
 * @brief Destroy and pop symbol table on the top of the stack
 * @return `True` on success, `False` on failure.
 * @pre Will set Error_Internal on error.
 */
bool symstack_pop();

/**
 * @brief Search the stack for symbol.
 *
 * Search from top to bottom of the stack for the first symbol table that contains
 * symbol with given name.
 * @param[in] sym_name Symbol to search for.
 * @return `Pointer to symbol table` containing symbol or `NULL` if the symbol doesn't exist.
 * @pre Will set Error_Internal on error.
 */
Symtable* symstack_search(const char* sym_name);

/**
 * @brief Search the stack for variable symbol.
 *
 * Search the stack from top to bottom and return the first VariableSymbol that has the same nane.
 * @param[in] var_name Name of the variable to search for.
 * @return Pointer to the VariableSymbol or NULL when not found.
 */
VariableSymbol* symstack_search_variable(const char* var_name);

/**
 * @brief Get the index of symtable on the stack.
 *
 * This function returns index of the given symtable from the bottom.
 * @param st Symtable to get the index of.
 * @return Index of the symtable.
 */
int symstack_index(Symtable* st);

#endif  // _SYMSTACK_H_
