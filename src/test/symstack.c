/**
 * @file test/symstack.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 25/11/2023
 * @brief Tester for SymStack
 */

#include "../symstack.h"
#include "test.h"


int main() {
    atexit(summary);

    FunctionSymbol fs;
    VariableSymbol vs;

    SymStack ss;

    suite("Test symstack_init") {
        test(symstack_init(&ss));
        test(got_error() == Error_None);
        clear_int_error();
    }

    suite("Test symstack_empty") {
        test(symstack_empty(&ss));
        symstack_push(&ss);
        test(!symstack_empty(&ss));
        symstack_clear(&ss);
        clear_int_error();
    }

    suite("Test symstack_push") {
        test(ss.size == 0);
        Symtable* st = symstack_push(&ss);
        test(got_error() == Error_None);
        test(st != NULL);
        test(ss.size == 1);
        st = symstack_push(&ss);
        test(got_error() == Error_None);
        test(st != NULL);
        test(ss.size == 2);
    }

    suite("Test symstack_clear") {
        symstack_clear(&ss);
        test(got_error() == Error_None);
        test(ss.size == 0);
        test(ss.top == NULL);
    }

    suite("Test symstack_free") {
        symstack_free(&ss);
        test(got_error() == Error_None);
        test(ss.size == 0);
        test(ss.top == NULL);
    }

    symstack_init(&ss);

    suite("Test symstack_top") {
        Symtable* s0 = symstack_push(&ss);
        Symtable* s1 = symstack_push(&ss);

        test(s1 == symstack_top(&ss));
        symstack_pop(&ss);
        test(s0 == symstack_top(&ss));

        symstack_clear(&ss);
    }

    suite("Test symstack_pop") {
        Symtable* s0 = symstack_push(&ss);
        symstack_push(&ss);

        test(symstack_pop(&ss));
        test(got_error() == Error_None);
        test(ss.size == 1);
        test(symstack_top(&ss) == s0);

        symstack_clear(&ss);
    }

    suite("Test symstack_search") {
        Symtable* s0 = symstack_push(&ss);
        symtable_insert_function(s0, "fun0", fs);
        symtable_insert_variable(s0, "var0", vs);
        Symtable* s1 = symstack_push(&ss);
        symtable_insert_function(s1, "fun1", fs);
        symtable_insert_variable(s1, "var1", vs);
        Symtable* s2 = symstack_push(&ss);
        symtable_insert_function(s2, "fun2", fs);
        symtable_insert_variable(s2, "var1", vs);
        Symtable* s3 = symstack_push(&ss);
        symtable_insert_variable(s3, "var3", vs);
        symtable_insert_variable(s3, "var0", vs);

        test(symstack_search(&ss, "var3") == s3);
        test(symstack_search(&ss, "var0") == s3);

        test(symstack_search(&ss, "var1") == s2);
        test(symstack_search(&ss, "fun2") == s2);

        test(symstack_search(&ss, "fun1") == s1);
        test(symstack_search(&ss, "var1") == s2);

        test(symstack_search(&ss, "fun0") == s0);

        symstack_pop(&ss);
        test(symstack_search(&ss, "var0") == s0);
        test(symstack_search(&ss, "var3") == NULL);
        test(got_error() == Error_None);
        test(symstack_search(&ss, "fun2") == s2);
        symstack_pop(&ss);
        test(symstack_search(&ss, "fun2") == NULL);
        test(got_error() == Error_None);
        test(symstack_search(&ss, "var1") == s1);

        symstack_clear(&ss);
    }

    symstack_free(&ss);

    return 0;
}
