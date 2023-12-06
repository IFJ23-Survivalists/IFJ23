/**
 * @note Project: Implementace překladače imperativního jazyka IFJ23
 * @file function_stack.h
 * @author Matúš Moravčík, xmorav48, VUT FIT
 * @date 30/11/2023
 * @brief A module for handling nested function calls.
 */

#ifndef _FUNCTION_STACK_H_
#define _FUNCTION_STACK_H_

#include <stdbool.h>
#include "expr_parser.h"
#include "string.h"
#include "symtable.h"

struct NTerm;

/**
 * @struct Stack
 * @brief Stack implemented as a single linked list. It is used for storing information about function when there are
 * nested function calls.
 */
typedef struct {
    struct StackNode* top; /**< Top of the stack */
} Stack;

/**
 * @struct StackNode
 * @brief Holds information about function.
 */
typedef struct StackNode {
    struct NTerm** param;
    char* name;      /**< Name of the function */
    int param_count; /**< Number of currently passed arguments */
    int capacity;
    struct StackNode* next; /**< Pointer to function that contains current one */
} StackNode;

/**
 * @brief Initialize an empty stack.
 * @param[out] stack The Stuck struct to initialize.
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
 * @param[out] stack The Stuck struct.
 */
void stack_pop(Stack* stack);

/**
 * @brief Dynamically allocate a new node and add it at the top of the stack.
 * @param[out] stack The Stuck struct.
 */
void stack_push(Stack* stack);

/**
 * @brief Free all memory asocieted with stack.
 * @param[out] stack The Stuck struct.
 * @param[in] param Expect already dynamically allocated `param`
 */
void stack_free(Stack* stack);

/**
 * @brief Insert parameter into `node`.
 * @param[out] node The node where will param be inserted.
 * @param[in] param Dynamically allocated Param
 * @return `true` if parameter was successfully inserted, otherwise `false`
 */
bool insert_param(StackNode* node, struct NTerm* param);

#endif  // _FUNCTION_STACK_H_
