/**
 * @note Project: Implementace překladače imperativního jazyka IFJ23
 * @file test/function_stack.c
 * @author Matúš Moravčík, xmorav48, VUT FIT
 * @date 01/12/2023
 * @brief Tester for function_stack.c
 */

#include "../function_stack.h"
#include "test.h"

int main() {
    atexit(summary);

    Stack stack;
    NTerm* exp = malloc(sizeof(NTerm));
    NTerm* rule = malloc(sizeof(NTerm));
    NTerm* id = malloc(sizeof(NTerm));

    exp->name = 'E';
    rule->name = '|';
    id->name = 'i';
    exp->code_name = NULL;
    rule->code_name = NULL;
    id->code_name = NULL;

    suite("Test stack_init") {
        stack_init(&stack);
        test(stack_top(&stack) == NULL);
    }

    suite("Test stack_push and insert_param") {
        stack_push(&stack);
        test(stack_top(&stack)->param_count == 0);
        insert_param(stack_top(&stack), exp);
        test(stack_top(&stack)->param_count == 1);
        test(stack_top(&stack)->param[0]->name == 'E');
        stack_push(&stack);
        insert_param(stack_top(&stack), rule);
        insert_param(stack_top(&stack), id);
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
