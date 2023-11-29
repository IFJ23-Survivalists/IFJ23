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
    function_symbol_init(&fs);
    VariableSymbol vs;
    variable_symbol_init(&vs);

    suite("Test symstack_init") {
        test(symstack_init());
        test(got_error() == Error_None);
        print_int_error_msg();
        clear_int_error();
    }

    suite("Test symstack_empty") {
        test(symstack_empty());
        symstack_push();
        test(!symstack_empty());
        symstack_clear();
        clear_int_error();
        print_int_error_msg();
        clear_int_error();
    }

    suite("Test symstack_push") {
        test(symstack_size() == 0);
        Symtable* st = symstack_push();
        print_int_error_msg();
        test(got_error() == Error_None);
        test(st != NULL);
        test(symstack_size() == 1);
        st = symstack_push();
        test(got_error() == Error_None);
        test(st != NULL);
        test(symstack_size() == 2);
        print_int_error_msg();
        clear_int_error();
    }

    suite("Test symstack_clear") {
        symstack_clear();
        test(got_error() == Error_None);
        test(symstack_size() == 0);
        test(symstack_top() == NULL);
        print_int_error_msg();
        clear_int_error();
    }

    suite("Test symstack_free") {
        symstack_free();
        test(got_error() == Error_None);
        print_int_error_msg();
        clear_int_error();
    }

    symstack_init();

    suite("Test symstack_top") {
        Symtable* s0 = symstack_push();
        Symtable* s1 = symstack_push();

        test(s1 == symstack_top());
        symstack_pop();
        test(s0 == symstack_top());

        symstack_clear();
        print_int_error_msg();
        clear_int_error();
    }

    suite("Test symstack_pop") {
        Symtable* s0 = symstack_push();
        symstack_push();

        test(symstack_pop());
        test(got_error() == Error_None);
        test(symstack_size() == 1);
        test(symstack_top() == s0);

        symstack_clear();
        print_int_error_msg();
        clear_int_error();
    }

    suite("Test symstack_search") {
        Symtable* s0 = symstack_push();
        symtable_insert_function(s0, "fun0", fs);
        symtable_insert_variable(s0, "var0", vs);
        Symtable* s1 = symstack_push();
        symtable_insert_function(s1, "fun1", fs);
        symtable_insert_variable(s1, "var1", vs);
        Symtable* s2 = symstack_push();
        symtable_insert_function(s2, "fun2", fs);
        symtable_insert_variable(s2, "var1", vs);
        Symtable* s3 = symstack_push();
        symtable_insert_variable(s3, "var3", vs);
        symtable_insert_variable(s3, "var0", vs);

        test(symstack_search("var3") == s3);
        test(symstack_search("var0") == s3);

        test(symstack_search("var1") == s2);
        test(symstack_search("fun2") == s2);

        test(symstack_search("fun1") == s1);
        test(symstack_search("var1") == s2);

        test(symstack_search("fun0") == s0);

        symstack_pop();
        test(symstack_search("var0") == s0);
        test(symstack_search("var3") == NULL);
        test(got_error() == Error_None);
        test(symstack_search("fun2") == s2);
        symstack_pop();
        test(symstack_search("fun2") == NULL);
        test(got_error() == Error_None);
        test(symstack_search("var1") == s1);

        symstack_clear();
        print_int_error_msg();
        clear_int_error();
    }

    symstack_free();

    return 0;
}
