/**
 * @file string_stack.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 25/09/2023
 * @brief A module for managing dynamically allocated strings in C.
 */

#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include "string.h"

struct StackNode;

/**
 * @struct Stack
 * @brief .
 */
typedef struct {
    struct StackNode* top;
} Stack;

typedef struct StackNode {
    String name;
    struct StackNode* next;
} StackNode;

/**
 * @brief Initialize an empty stack.
 * @param[in] stack The Stuck struct to initialize.
 */
void stack_init(Stack* stack);

/**
 * @brief Returns `true` if stack is empty, otherwise `false`.
 * @param[in] stack The Stuck struct.
 * @return `true` if stack is empty, otherwise `false`.
 */
bool stack_empty(const Stack* stack);

/**
 * @brief Return the topmost `StackNode` in stack.
 * @param[in] stack The Stuck struct.
 * @return The topmost `StackNode` in stack.
 */
StackNode* stack_top(const Stack* stack);

/**
 * @brief Remove the topmost node from stack.
 * @param[in] stack The Stuck struct.
 */
void stack_pop(Stack* stack);

/**
 * @brief Dynamically allocate a new node and add it at the top of the stack.
 * @param[in] stack The Stuck struct.
 * @param[in] name The name of the node.
 */
void stack_push(Stack* stack, String name);

/**
 * @brief Free all memory asocieted with stack.
 * @param[in] stack The Stuck struct.
 */
void stack_free(Stack* stack);

#endif
