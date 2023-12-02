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
    String a = string_from_c_str("a");
    String b = string_from_c_str("b");
    String c = string_from_c_str("c");

    suite("Test stack_init") {
        stack_init(&stack);
        test(stack_top(&stack) == NULL);
    }

    suite("Test stack_push") {
        stack_push(&stack, a, NULL);
        stack_push(&stack, b, NULL);
        stack_push(&stack, c, NULL);
        test(strcmp(stack_top(&stack)->name.data, c.data) == 0);
        test(strcmp(stack_top(&stack)->name.data, a.data) != 0);
        test(strcmp(stack_top(&stack)->name.data, b.data) != 0);
        test(stack_top(&stack)->processed_args == 0);
    }

    suite("Test stack_pop") {
        stack_pop(&stack);
        test(strcmp(stack_top(&stack)->name.data, c.data) != 0);
        test(strcmp(stack_top(&stack)->name.data, b.data) == 0);
    }

    suite("Test stack_empty") {
        stack_pop(&stack);
        stack_pop(&stack);
        test(stack_empty(&stack));
        stack_push(&stack, a, NULL);
        test(!stack_empty(&stack));
    }

    suite("Test stack_top") {
        stack_pop(&stack);
        test(stack_top(&stack) == NULL);
        stack_push(&stack, a, NULL);
        test(strcmp(stack_top(&stack)->name.data, a.data) == 0);
        stack_push(&stack, b, NULL);
        test(strcmp(stack_top(&stack)->name.data, b.data) == 0);
    }

    stack_free(&stack);
    string_free(&a);
    string_free(&b);
    string_free(&c);

    return 0;
}
