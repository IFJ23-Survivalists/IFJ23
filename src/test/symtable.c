/**
 * @file test/symtable.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 12/10/2023
 * @brief Tester for symtable.h
 */

#include "../symtable.h"
#include "test.h"
#include <string.h>

int main() {
    atexit(summary);

    FunctionParameter params;
    Symtable symtable;

    suite("Test function_parameter_init") {
        function_parameter_init(&params);
    }

    suite("Test symtable_init") {
        symtable_init(&symtable);
        test(!symtable.root);
    }

    suite("Test symtable_insert_function") {
        FunctionSymbol foo;
        function_symbol_init(&foo);
        foo.return_value_type = DataType_Nil;

        test(symtable_insert_function(&symtable, "foo", foo));

        FunctionSymbol foo2;
        function_symbol_init(&foo2);
        foo2.return_value_type = DataType_Double;
        function_symbol_emplace_param(&foo2, DataType_Double, NULL, "p1");
        function_symbol_emplace_param(&foo2, DataType_Int, "abc", "p2");
        test(!symtable_insert_function(&symtable, "foo", foo2));
        test(symtable_insert_function(&symtable, "foo2", foo2));
    }

    suite("Test symtable_insert_variable") {
        VariableSymbol bar;
        bar.type = DataType_Int;
        bar.is_defined = true;
        bar.allow_modification = false;

        test(symtable_insert_variable(&symtable, "bar", bar));

        VariableSymbol baz;
        baz.type = DataType_Double;
        baz.is_defined = false;
        baz.allow_modification = true;
        test(!symtable_insert_variable(&symtable, "bar", baz));
        test(symtable_insert_variable(&symtable, "baz", baz));

    }

    suite("Test symtable_get_function") {
        test(!symtable_get_function(&symtable, ""));
        test(!symtable_get_function(&symtable, "unknown"));
        test(!symtable_get_function(&symtable, "bar"));
        test(!symtable_get_function(&symtable, "baz"));

        FunctionSymbol *func = symtable_get_function(&symtable, "foo");
        test(func);
        test(func->return_value_type == DataType_Nil);

        FunctionSymbol* func2 = symtable_get_function(&symtable, "foo2");
        test(func2);
        test(func != func2);
        test(func2->return_value_type == DataType_Double);
        test(func2->param_count == 2);
        test(func2->params[0].type == DataType_Double);
        test(func2->params[0].oname.data == NULL);
        test(strcmp(func2->params[0].iname.data, "p1") == 0);
        test(func2->params[1].type == DataType_Int);
        test(strcmp(func2->params[1].oname.data, "abc") == 0);
        test(strcmp(func2->params[1].iname.data, "p2") == 0);
    }

    suite("Test symtable_get_variable") {
        test(!symtable_get_variable(&symtable, ""));
        test(!symtable_get_variable(&symtable, "unknown"));
        test(!symtable_get_variable(&symtable, "foo"));
        test(!symtable_get_variable(&symtable, "foo2"));

        VariableSymbol *var = symtable_get_variable(&symtable, "bar");
        test(var);
        test(var->type == DataType_Int);
        test(var->is_defined == true);
        test(var->allow_modification == false);

        VariableSymbol *var2 = symtable_get_variable(&symtable, "baz");
        test(var);
        test(var != var2);
        test(var2->type == DataType_Double);
        test(var2->is_defined == false);
        test(var2->allow_modification == true);
    }

    suite("Test Balance") {
        test((symtable.root->left->height - symtable.root->right->height) <= 1);
    }

    suite("Test symtable_get_symbol_type") {
        test(!symtable_get_symbol_type(&symtable, ""));
        test(!symtable_get_symbol_type(&symtable, "unknown"));
        NodeType *node = symtable_get_symbol_type(&symtable, "foo");
        test(node);
        test(*node == NodeType_Function);

        node = symtable_get_symbol_type(&symtable, "foo2");
        test(node);
        test(*node == NodeType_Function);

        node = symtable_get_symbol_type(&symtable, "bar");
        test(node);
        test(*node == NodeType_Variable);

        node = symtable_get_symbol_type(&symtable, "baz");
        test(node);
        test(*node == NodeType_Variable);
    }

    suite("Test symtable_free") {
        symtable_free(&symtable);
        test(!symtable.root);
    }

    return 0;
}
