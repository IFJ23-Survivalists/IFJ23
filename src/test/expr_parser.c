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

#define TEST_VALID_EXPRESSION(str, dt)               \
    do {                                             \
        scanner_init_str(str);                       \
        parser_next_token();                         \
        test(expr_parser_begin(&expr_data) == true); \
        test(expr_data.type == dt);                  \
        scanner_free();                              \
    } while (0)

#define TEST_INVALID_EXPRESSION(str, return_code)     \
    do {                                              \
        scanner_init_str(str);                        \
        parser_next_token();                          \
        test(expr_parser_begin(&expr_data) == false); \
        test(got_error() == return_code);             \
        scanner_free();                               \
        set_error(Error_None);                        \
    } while (0)

void insert_variables() {
    VariableSymbol a;
    VariableSymbol b;
    VariableSymbol name;
    VariableSymbol y;

    variable_symbol_init(&a);
    variable_symbol_init(&b);
    variable_symbol_init(&name);
    variable_symbol_init(&y);

    a.is_initialized = true;
    b.is_initialized = true;
    name.is_initialized = true;
    y.is_initialized = true;

    a.type = DataType_Int;
    b.type = DataType_Double;
    name.type = DataType_String;
    y.type = DataType_MaybeInt;

    symtable_insert_variable(symstack_top(), "a", a);
    symtable_insert_variable(symstack_top(), "b", b);
    symtable_insert_variable(symstack_top(), "name", name);
    symtable_insert_variable(symstack_top(), "y", y);
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

    function_symbol_emplace_param(&many, DataType_Int, "int", "x");
    function_symbol_emplace_param(&many, DataType_Double, NULL, "d");
    function_symbol_emplace_param(&many, DataType_MaybeInt, NULL, "mint");
    function_symbol_emplace_param(&many, DataType_MaybeDouble, NULL, "md");

    function_symbol_emplace_param(&str, DataType_String, NULL, "a");
    function_symbol_emplace_param(&str, DataType_String, NULL, "b");

    symtable_insert_function(symstack_top(), "fn", fn);
    symtable_insert_function(symstack_top(), "many", many);
    symtable_insert_function(symstack_top(), "x", x);
    symtable_insert_function(symstack_top(), "str", str);
}

int main() {
    atexit(summary);

    parser_init();
    insert_variables();
    insert_functions();

    Data expr_data;

    suite("Test valid arithmetic expressions") {
        TEST_VALID_EXPRESSION("nil", DataType_Undefined);
        TEST_VALID_EXPRESSION("1", DataType_Int);
        TEST_VALID_EXPRESSION("\"adsf\"", DataType_String);
        TEST_VALID_EXPRESSION("1 + 2 * 3", DataType_Int);
        TEST_VALID_EXPRESSION("1.2 + 2", DataType_Double);
        TEST_VALID_EXPRESSION("\"fsd\" + \"45\"", DataType_String);
        TEST_VALID_EXPRESSION("a * 1.4", DataType_Int);
        TEST_VALID_EXPRESSION("b / 1.4", DataType_Double);
        TEST_VALID_EXPRESSION("b - 1", DataType_Double);
        TEST_VALID_EXPRESSION("y! - (-a)", DataType_Int);
        TEST_VALID_EXPRESSION("((((((1) +1) -1) +1) -1) *0.1)", DataType_Int);
    }

    suite("Test valid logic expressions") {
        TEST_VALID_EXPRESSION("1 < 0", DataType_Bool);
        TEST_VALID_EXPRESSION("\"adsf\" == \"fdsaf\"", DataType_Bool);
        TEST_VALID_EXPRESSION("!true", DataType_Bool);
        TEST_VALID_EXPRESSION("true == true", DataType_Bool);
        TEST_VALID_EXPRESSION("1 == 1 == 1", DataType_Bool);  // end when encounter second '=='
        TEST_VALID_EXPRESSION("1 > 1.0 ", DataType_Bool);
        TEST_VALID_EXPRESSION("1.7 < b ", DataType_Bool);
        TEST_VALID_EXPRESSION("(y!) != (-a) *2", DataType_Bool);
    }

    suite("Test valid function expressions") {
        TEST_VALID_EXPRESSION("fn(x: 1, y: 1.1)", DataType_MaybeDouble);
        TEST_VALID_EXPRESSION("1 + fn(x: a + 2 / 3, y: b)! * 2", DataType_Double);
        TEST_VALID_EXPRESSION("1 + many(int: a, b, y, 5.5) * 2", DataType_Double);
        TEST_VALID_EXPRESSION("x() != x() * 2", DataType_Bool);
        TEST_VALID_EXPRESSION("str(name, name) + \"String\"", DataType_String);
    }

    suite("Test valid nil coalescing") {
        TEST_VALID_EXPRESSION("y ?? 0", DataType_Int);
        TEST_VALID_EXPRESSION("fn(x:1, y:2.1) ?? 4.5 ?? 0.5", DataType_Double);
        TEST_VALID_EXPRESSION("y ?? 1+ 2 + a - (-6)", DataType_Int);
    }

    suite("Test invalid arithmetic expressions") {
        TEST_INVALID_EXPRESSION("+1", Error_Syntax);
        TEST_INVALID_EXPRESSION("(-1+1", Error_Syntax);
        TEST_INVALID_EXPRESSION(" 1+, a", Error_Syntax);
        TEST_INVALID_EXPRESSION(" (1, a)", Error_Syntax);
        TEST_INVALID_EXPRESSION(" 1!", Error_Operation);
        TEST_INVALID_EXPRESSION("a + b", Error_Operation);
        // TEST_INVALID_EXPRESSION("1 + nil", Error_UnknownType);
    }

    parser_free();
    return 0;
}
