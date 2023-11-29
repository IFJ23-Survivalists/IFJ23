/**
 * @brief Implementation of dynamically growing array.
 * @file pushdown.h
 * @author Jakub Kloub, xkloub03, FIT VUT
 * @author Matúš Moravčík, xmorav48, FIT VUT
 * @date 10/11/2023
 */
#ifndef _PUSHDOWN_H_
#define _PUSHDOWN_H_

#include <stdbool.h>
#include <stddef.h>
#include "expr_parser.h"
#include "scanner.h"
#include "string.h"

#define MAX_RULE_LENGTH 4

// Forward declaration
struct Nterm;

typedef struct PushdownItem {
    Token* terminal;     /**< Terminal. */
    struct NTerm* nterm; /**< Non terminal */
    char name;
    struct PushdownItem* next;
    struct PushdownItem* prev;
} PushdownItem;

typedef struct Pushdown {
    PushdownItem* first;
    PushdownItem* last;
} Pushdown;

/**
 * @brief Initialize pushdown.
 * @param pushdown Pushdown to be initialized.
 */
void pushdown_init(Pushdown* pushdown);

/**
 * @brief Free all memory used by pushdown.
 * @param pushdown Pushdown to be destroyed.
 */
void pushdown_destroy(Pushdown* pushdown);

/**
 * @brief Adds a new item to the back of the Pushdown structure.
 * @param pushdown Pointer to the Pushdown structure.
 * @param value Pointer to the PushdownItem to be added.
 */
void pushdown_insert_last(Pushdown* pushdown, PushdownItem* value);

/**
 * @brief Inserts a new `value` at the specified index in the Pushdown structure.
 * @param pushdown Pointer to the Pushdown structure.
 * @param item The item where will be new `value` inserted.
 * @param value Pointer to the PushdownItem to be inserted.
 */
void pushdown_insert_after(Pushdown* pushdown, PushdownItem* item, PushdownItem* value);

/**
 * @brief Returns the item at the back of the Pushdown structure.
 * @param pushdown Pointer to the Pushdown structure.
 * @return The item at the back of the pushdown.
 */
PushdownItem* pushdown_last(const Pushdown* pushdown);

/**
 * @brief Returns the item at the back of the Pushdown structure.
 * @param item Pointer to the current item.
 * @return The `PushdownItem` after `item` or NULL if `item` is last.
 */
PushdownItem* pushdown_next(const PushdownItem* item);

/**
 * @brief Search for the first occurence of `PushdownItem` with `name` from the back of the pushdown.
 * @param pushdown Pointer to the Pushdown structure.
 * @param name The name of `PushdownItem`.
 * @return The pointer to `PushdownItem` with a specific name if found, otherwise `NULL`.
 */
PushdownItem* pushdown_search_name(Pushdown* pushdown, char name);

/**
 * @brief Retrieves the `PushdownItem` of the topmost terminal in the Pushdown structure.
 * @param[in] pushdown Pointer to the Pushdown structure.
 * @return `PushdownItem` of topmost terminal in pushdown, otherwise NULL.
 */
PushdownItem* pushdown_search_terminal(struct Pushdown* pushdown);

/**
 * @brief Remove all items from current to the end of the pushdown.
 * @param pushdown Pointer to the Pushdown structure.
 * @param item The item from which all items to the last will be deleted.
 */
void pushdown_remove_all_from_current(Pushdown* pushdown, PushdownItem* item);

/**
 * @brief Creates a new PushdownItem object with the given Token and NTerm pointers.
 *
 * This function dynamically allocates memory for a new PushdownItem, initializes its fields with the provided
 * Token and NTerm pointers, and sets the default name for the PushdownItem to '|' indicating the end of a rule.
 *
 * @param term Pointer to the Token associated with the PushdownItem.
 * @param nterm Pointer to the NTerm associated with the PushdownItem.
 * @return A pointer to the newly created PushdownItem.
 */
PushdownItem* create_pushdown_item(Token* term, struct NTerm* nterm);

#endif  // _PUSHDOWN_H_
