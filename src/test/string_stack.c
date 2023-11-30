/**
 * @file test/string.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 13/10/2023
 * @brief Tester for string.h
 */

#include "../string_stack.h"
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
        stack_push(&stack, a);
        stack_push(&stack, b);
        stack_push(&stack, c);
        test(strcmp(stack_top(&stack)->name.data, c.data) == 0);
        test(strcmp(stack_top(&stack)->name.data, a.data) != 0);
        test(strcmp(stack_top(&stack)->name.data, b.data) != 0);
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
        stack_push(&stack, a);
        test(!stack_empty(&stack));
    }

    suite("Test stack_top") {
        stack_pop(&stack);
        test(stack_top(&stack) == NULL);
        stack_push(&stack, a);
        test(strcmp(stack_top(&stack)->name.data, a.data) == 0);
        stack_push(&stack, b);
        test(strcmp(stack_top(&stack)->name.data, b.data) == 0);
    }

    stack_free(&stack);
    string_free(&a);
    string_free(&b);
    string_free(&c);

    return 0;
}
