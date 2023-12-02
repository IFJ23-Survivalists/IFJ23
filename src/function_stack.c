/**
 * @file function_stack.c
 * @author Matúš Moravčík, xmorav48, VUT FIT
 * @date 30/11/2023
 * @brief A module for handling nested function calls.
 */

#include "function_stack.h"
#include "error.h"
#include "stdbool.h"
#include "string.h"
#include "symtable.h"

void stack_init(Stack* stack) {
    stack->top = NULL;
}

bool stack_empty(const Stack* stack) {
    return stack->top == NULL;
}

StackNode* stack_top(const Stack* stack) {
    return stack->top;
}

void stack_pop(Stack* stack) {
    if (!stack_empty(stack)) {
        StackNode* to_delete = stack->top;
        stack->top = to_delete->next;
        free(to_delete);
    }
}

void stack_push(Stack* stack, String name, FunctionSymbol* fn) {
    StackNode* node = malloc(sizeof(StackNode));
    node->fn = fn;
    node->name = name;
    node->processed_args = 0;
    node->next = stack->top;
    stack->top = node;
}

void stack_free(Stack* stack) {
    while (!stack_empty(stack))
        stack_pop(stack);
}
