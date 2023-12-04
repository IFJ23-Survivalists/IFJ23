/**
 * @file test/string.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 13/10/2023
 * @brief Tester for string.h
 */

#include "../function_stack.h"
#include <stdbool.h>
#include <string.h>
#include "test.h"

int main() {
    atexit(summary);

    Stack stack;
    NTerm exp = {.name = 'E'};
    NTerm rule = {.name = '|'};
    NTerm id = {.name = 'i'};

    suite("Test stack_init") {
        stack_init(&stack);
        test(stack_top(&stack) == NULL);
    }

    suite("Test stack_push and insert_param") {
        stack_push(&stack);
        test(stack_top(&stack)->param_count == 0);
        insert_param(stack_top(&stack), &exp);
        test(stack_top(&stack)->param_count == 1);
        test(stack_top(&stack)->param[0]->name == 'E');
        stack_push(&stack);
        insert_param(stack_top(&stack), &rule);
        insert_param(stack_top(&stack), &id);
        test(stack_top(&stack)->param_count == 2);
        test(stack_top(&stack)->param[0]->name == '|');
        test(stack_top(&stack)->param[1]->name == 'i');
    }

    suite("Test stack_pop") {
        stack_pop(&stack);
        test(stack_top(&stack)->param_count == 1);
        test(stack_top(&stack)->param[0]->name == 'E');
        stack_pop(&stack);
        test(stack_top(&stack) == NULL);
    }

    suite("Test stack_empty") {
        test(stack_empty(&stack));
        stack_push(&stack);
        test(!stack_empty(&stack));
    }

    suite("Test stack_top") {
        test(stack_top(&stack) != NULL);
        stack_pop(&stack);
        test(stack_top(&stack) == NULL);
        stack_push(&stack);
        test(stack_top(&stack) != NULL);
        test(stack_top(&stack)->param_count == 0);
    }

    stack_free(&stack);

    return 0;
}
