/**
 * @file string_stack.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 25/09/2023
 * @brief A module for managing dynamically allocated strings in C.
 */

#include "string_stack.h"
#include "error.h"
#include "stdbool.h"
#include "string.h"

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

void stack_push(Stack* stack, String name) {
    StackNode* node = malloc(sizeof(StackNode));
    node->name = name;
    node->next = stack->top;
    stack->top = node;
}

void stack_free(Stack* stack) {
    while (!stack_empty(stack))
        stack_pop(stack);
}
