/**
 * @file test/expr_parser.c
 * @author Matúš Moravčík, xmorav48, VUT FIT
 * @date 27/11/2023
 * @brief Tester for expr_parser.h
 */

#include "../expr_parser.h"
#include <string.h>
#include "../parser.h"
#include "../scanner.h"
#include "test.h"

void insert_variables() {
    VariableSymbol a;
    VariableSymbol b;
    VariableSymbol name;
    VariableSymbol y;

    variable_symbol_init(&a);
    variable_symbol_init(&b);
    variable_symbol_init(&name);
    variable_symbol_init(&y);

    a.is_defined = true;
    b.is_defined = true;
    name.is_defined = true;
    y.is_defined = true;

    a.type = DataType_Int;
    b.type = DataType_Double;
    name.type = DataType_String;
    y.type = DataType_MaybeInt;

    symtable_insert_variable(symstack_top(&g_symstack), "a", a);
    symtable_insert_variable(symstack_top(&g_symstack), "b", b);
    symtable_insert_variable(symstack_top(&g_symstack), "name", name);
    symtable_insert_variable(symstack_top(&g_symstack), "y", y);
}

void insert_functions() {
    FunctionSymbol fn;
    FunctionSymbol many;
    FunctionSymbol x;
    FunctionSymbol str;

    function_symbol_init(&fn);
    function_symbol_init(&many);
    function_symbol_init(&x);
    function_symbol_init(&str);

    fn.return_value_type = DataType_MaybeDouble;
    many.return_value_type = DataType_Double;
    x.return_value_type = DataType_Int;
    str.return_value_type = DataType_String;

    function_symbol_emplace_param(&fn, DataType_Int, "x", "x");
    function_symbol_emplace_param(&fn, DataType_Double, "y", "y");

    function_symbol_emplace_param(&many, DataType_Int, NULL, "int");
    function_symbol_emplace_param(&many, DataType_Double, NULL, "d");
    function_symbol_emplace_param(&many, DataType_MaybeInt, NULL, "mint");
    function_symbol_emplace_param(&many, DataType_MaybeDouble, NULL, "md");

    function_symbol_emplace_param(&str, DataType_String, NULL, "a");
    function_symbol_emplace_param(&str, DataType_String, NULL, "b");

    symtable_insert_function(symstack_top(&g_symstack), "fn", fn);
    symtable_insert_function(symstack_top(&g_symstack), "many", many);
    symtable_insert_function(symstack_top(&g_symstack), "x", x);
    symtable_insert_function(symstack_top(&g_symstack), "str", str);
}

int main() {
    atexit(summary);

    FILE* valid_syn_fp = fopen("test/expr_parser_files/valid_syntax.swift", "r+");
    FILE* valid_sem_fp = fopen("test/expr_parser_files/valid_semantic.swift", "r+");

    // for semantic tests
    DataType expected_value_types[] = {
        DataType_Double, DataType_String, DataType_Bool,   DataType_Int,         DataType_String,
        DataType_Double, DataType_Bool,   DataType_Double, DataType_MaybeDouble,
    };

    if (!valid_sem_fp || !valid_syn_fp) {
        eprint("Unable to read file\n");
        return 99;
    }

    scanner_init(valid_syn_fp);
    parser_init();

    insert_variables();
    insert_functions();

    Token* starting_token = parser_next_token();

    suite("Test syntactically valid expressions") {
        while ((*starting_token).type != Token_EOF) {
            Data expr_data;
            test(expr_parser_begin(&expr_data) == true);
            starting_token = parser_next_token();
        }
    }
    scanner_free();

    scanner_init(valid_sem_fp);

    starting_token = parser_next_token();
    suite("Test semanticly valid expressions") {
        int idx = 0;
        while ((*starting_token).type != Token_EOF) {
            Data expr_data;
            expr_parser_begin(&expr_data);
            test(expr_data.type == expected_value_types[idx++]);
            starting_token = parser_next_token();
        }
    }

    scanner_free();

    parser_free();
    scanner_free();
    return 0;
}
