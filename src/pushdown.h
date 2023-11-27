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

// default pushdown capacity (allocated space)
#define DEFAULT_CAPACITY 4

// Forward declaration
struct Nterm;

typedef struct PushdownItem {
    Token* terminal;     /**< Terminal. */
    struct NTerm* nterm; /**< Non terminal */
    char name;
} PushdownItem;

typedef struct Pushdown {
    PushdownItem* data;
    size_t size;
    size_t capacity;
} Pushdown;

/**
 * @brief Determine if dynamic array should be resized.
 * @param size Current size of the array
 * @param capacity Current capacity of the array.
 * @return Boolean signaling that the array should be resized or not.
 */
bool pushdown_should_resize(size_t size, size_t capacity);

/**
 * @brief Handles resizing of a dynamic array to accommodate additional elements.
 *
 * This function checks if resizing is needed based on the current size and capacity of the dynamic array.
 * If resizing is required, it doubles the capacity and reallocates memory for the array. If the reallocation fails,
 * it sets an integer error state and returns false.
 *
 * @param p_data A pointer to the pointer to the dynamic array.
 * @param sizeof_item The size of each element in the array.
 * @param size The current size of the array.
 * @param capacity A pointer to the current capacity of the array.
 * @return true if resizing is successful or not needed, false if reallocation fails.
 */
bool handle_resize(void** p_data, size_t sizeof_item, size_t size, size_t* capacity);

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
void pushdown_push_back(Pushdown* pushdown, PushdownItem* value);

/**
 * @brief Inserts a new `value` at the specified index in the Pushdown structure.
 * @param pushdown Pointer to the Pushdown structure.
 * @param index The index at which the new item should be inserted.
 * @param value Pointer to the PushdownItem to be inserted.
 */
void pushdown_insert(Pushdown* pushdown, size_t index, PushdownItem* value);

/**
 * @brief Returns the item at the back of the Pushdown structure.
 * @param pushdown Pointer to the Pushdown structure.
 * @return The item at the back of the pushdown.
 */
PushdownItem pushdown_back(const Pushdown* pushdown);

/**
 * @brief Returns the item at the specified index in the Pushdown structure.
 * @param pushdown Pointer to the Pushdown structure.
 * @param idx The index of the item to retrieve.
 * @return The item at the specified index in the pushdown.
 */
PushdownItem pushdown_at(Pushdown* pushdown, size_t idx);

/**
 * @brief Reduces the size of the Pushdown structure to the specified new size.
 * @param pushdown Pointer to the Pushdown structure.
 * @param new_size The new size to which the pushdown should be reduced.
 */
void pushdown_reduce_size(Pushdown* pushdown, size_t new_size);

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

/**
 * @brief Sets the name field of a PushdownItem to the specified character.
 * @param item Pointer to the PushdownItem.
 * @param name The character to set as the name for the PushdownItem.
 * @return A pointer to the modified PushdownItem.
 */
PushdownItem* set_name(PushdownItem* item, char name);

#endif  // _PUSHDOWN_H_
