/**
 * @note Project: Implementace překladače imperativního jazyka IFJ23
 * @brief Implementation of dynamically growing array.
 * @file pushdown.h
 * @author Jakub Kloub, xkloub03, FIT VUT
 * @author Matúš Moravčík, xmorav48, FIT VUT
 * @date 23/11/2023
 */
#ifndef _PUSHDOWN_H_
#define _PUSHDOWN_H_

#include <stdbool.h>
#include <stddef.h>
#include "expr_parser.h"
#include "scanner.h"
#include "string.h"

#define MAX_RULE_LENGTH 4

/**
 * @struct PushdownItem
 * @brief Represent one item in `Pushdown`. It can be terminal, nonterminal or rule end marker.
 */
typedef struct PushdownItem {
    Token* term;               /**< Terminal. */
    struct NTerm* nterm;       /**< Non-terminal */
    char name;                 /**< Name of a given terminal or nonterminal or rule end marker */
    struct PushdownItem* next; /**< Reference to next `PushdownItem` */
    struct PushdownItem* prev; /**< Reference to previous `PushdownItem` */
} PushdownItem;

/**
 * @struct PushdownItem
 * @brief Hold all terminals, nonterminals and rule end markers that are step by step reduced to one nonterminal. It is
 * implemented as a double linked list.
 */
typedef struct Pushdown {
    PushdownItem* first; /**< Reference to the most bottom item. */
    PushdownItem* last;  /**< Reference to the topmost item. */
} Pushdown;

/**
 * @brief Initialize pushdown.
 * @param [out] pushdown Pushdown to be initialized.
 */
void pushdown_init(Pushdown* pushdown);

/**
 * @brief Free all memory used by pushdown.
 * @param [out] pushdown Pushdown to be destroyed.
 */
void pushdown_free(Pushdown* pushdown);

/**
 * @brief Adds a new item to the back of the Pushdown structure.
 * @param [out] pushdown Pointer to the Pushdown structure.
 * @param [in] value Pointer to the PushdownItem to be added.
 */
void pushdown_insert_last(Pushdown* pushdown, PushdownItem* value);

/**
 * @brief Inserts a new `value` at the specified index in the Pushdown structure.
 * @param [out] pushdown Pointer to the Pushdown structure.
 * @param [in] item The item where will be new `value` inserted.
 * @param [in] value Pointer to the PushdownItem to be inserted.
 */
void pushdown_insert_after(Pushdown* pushdown, PushdownItem* item, PushdownItem* value);

/**
 * @brief Returns the item at the back of the Pushdown structure.
 * @param [in] pushdown Pointer to the Pushdown structure.
 * @return The item at the back of the pushdown.
 */
PushdownItem* pushdown_last(const Pushdown* pushdown);

/**
 * @brief Returns the item at the back of the Pushdown structure.
 * @param [in] item Pointer to the current item.
 * @return The `PushdownItem` after `item` or NULL if `item` is last.
 */
PushdownItem* pushdown_next(const PushdownItem* item);

/**
 * @brief Search for the first occurence of `PushdownItem` with `name` from the back of the pushdown.
 * @param [in] pushdown Pointer to the Pushdown structure.
 * @param [in] name The name of `PushdownItem`.
 * @return The pointer to `PushdownItem` with a specific name if found, otherwise `NULL`.
 */
PushdownItem* pushdown_search_name(Pushdown* pushdown, const char name);

/**
 * @brief Retrieves the `PushdownItem` of the topmost terminal in the Pushdown structure.
 * @param[in] pushdown Pointer to the Pushdown structure.
 * @return `PushdownItem` of topmost terminal in pushdown, otherwise NULL.
 */
PushdownItem* pushdown_search_terminal(Pushdown* pushdown);

/**
 * @brief Remove all items from current to the end of the pushdown.
 * @param [out] pushdown Pointer to the Pushdown structure.
 * @param [in] item The item from which all items to the last will be deleted.
 */
void pushdown_remove_all_from_current(Pushdown* pushdown, PushdownItem* item);

/**
 * @brief Creates a new PushdownItem object with the given Token and NTerm pointers. This function dynamically allocates
 * memory for a new PushdownItem, initializes its fields with the provided Token and NTerm pointers, and sets the
 * default name for the PushdownItem to '|' indicating the end of a rule.
 * @param [in] term Pointer to the Token associated with the PushdownItem.
 * @param [in] nterm Pointer to the NTerm associated with the PushdownItem.
 * @return A pointer to the newly created PushdownItem.
 */
PushdownItem* create_pushdown_item(Token* term, struct NTerm* nterm);

#endif  // _PUSHDOWN_H_
