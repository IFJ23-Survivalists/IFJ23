/**
 * @file test/expr_parser.c
 * @author Matúš Moravčík, xmorav48, VUT FIT
 * @date 27/11/2023
 * @brief Tester for expr_parser.h
 */

#include "../expr_parser.h"
#include <string.h>
#include "../codegen.h"
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

#define INSERT_VARIABLE(name, dt)                              \
    do {                                                       \
        VariableSymbol name;                                   \
        variable_symbol_init(&name);                           \
        name.code_frame = Frame_Local;                         \
        string_concat_c_str(&name.code_name, #name);           \
        name.is_initialized = true;                            \
        name.type = dt;                                        \
        symtable_insert_variable(symstack_top(), #name, name); \
    } while (0)

#define INSERT_FUNCTION_WITH_ARGS(name, dt, ...)                                                       \
    do {                                                                                               \
        FunctionSymbol name;                                                                           \
        function_symbol_init(&name);                                                                   \
        name.return_value_type = dt;                                                                   \
        string_concat_c_str(&name.code_name, #name);                                                   \
        Param params[] = {__VA_ARGS__};                                                                \
        for (size_t i = 0; i < sizeof(params) / sizeof(params[0]); i++) {                              \
            function_symbol_emplace_param(&name, (params[i]).param_dt, (params[i]).param_name, #name); \
        }                                                                                              \
        symtable_insert_function(symstack_top(), #name, name);                                         \
    } while (0)

#define INSERT_FUNCTION_WITHOUT_ARGS(name, dt)                 \
    do {                                                       \
        FunctionSymbol name;                                   \
        function_symbol_init(&name);                           \
        name.return_value_type = dt;                           \
        symtable_insert_function(symstack_top(), #name, name); \
    } while (0)

typedef struct {
    char* param_name;
    DataType param_dt;
} Param;

int main() {
    atexit(summary);
    Data expr_data;
    CodeBuf buf;

    parser_init();
    code_buf_init(&buf);
    code_buf_set(&buf);

    INSERT_FUNCTION_WITHOUT_ARGS(x, DataType_Int);
    INSERT_FUNCTION_WITHOUT_ARGS(n, DataType_Undefined);
    INSERT_FUNCTION_WITH_ARGS(fn, DataType_MaybeDouble, {.param_name = "x", .param_dt = DataType_Int},
                              {.param_name = "y", .param_dt = DataType_Double});
    INSERT_FUNCTION_WITH_ARGS(many, DataType_Double, {.param_name = "int", .param_dt = DataType_Int},
                              {.param_name = NULL, .param_dt = DataType_Double},
                              {.param_name = NULL, .param_dt = DataType_MaybeInt},
                              {.param_name = NULL, .param_dt = DataType_MaybeDouble});
    INSERT_FUNCTION_WITH_ARGS(str, DataType_String, {.param_name = NULL, .param_dt = DataType_String},
                              {.param_name = NULL, .param_dt = DataType_String});
    INSERT_FUNCTION_WITH_ARGS(one, DataType_Int, {.param_name = "x", .param_dt = DataType_Int});

    INSERT_VARIABLE(a, DataType_Int);
    INSERT_VARIABLE(b, DataType_Double);
    INSERT_VARIABLE(name, DataType_String);
    INSERT_VARIABLE(y, DataType_MaybeInt);
    INSERT_VARIABLE(bl, DataType_MaybeBool);
    INSERT_VARIABLE(s, DataType_MaybeString);

    suite("Test valid arithmetic expressions") {
        TEST_VALID_EXPRESSION("nil", DataType_Undefined);
        TEST_VALID_EXPRESSION("(a)", DataType_Int);
        TEST_VALID_EXPRESSION("(1)", DataType_Int);
        TEST_VALID_EXPRESSION("\"adsf\"", DataType_String);
        TEST_VALID_EXPRESSION("1 + 2 * 3", DataType_Int);
        TEST_VALID_EXPRESSION("1.2 + 2", DataType_Double);
        TEST_VALID_EXPRESSION("1 - 1.4", DataType_Double);
        TEST_VALID_EXPRESSION("\"fsd\" + \"45\"", DataType_String);
        TEST_VALID_EXPRESSION("a * 1.4", DataType_Int);
        TEST_VALID_EXPRESSION("b / 1.4", DataType_Double);
        TEST_VALID_EXPRESSION("b - 1", DataType_Double);
        TEST_VALID_EXPRESSION("y! - (-a)", DataType_Int);
        TEST_VALID_EXPRESSION("((((((1) +1) -1) +1) -1) *0.1)", DataType_Double);
    }

    suite("Test valid logic expressions") {
        TEST_VALID_EXPRESSION("!true", DataType_Bool);
        TEST_VALID_EXPRESSION("bl!", DataType_Bool);
        TEST_VALID_EXPRESSION("1 <= 0.0", DataType_Bool);
        TEST_VALID_EXPRESSION("\"adsf\" == \"fdsaf\"", DataType_Bool);
        TEST_VALID_EXPRESSION("!true", DataType_Bool);
        TEST_VALID_EXPRESSION("true == true", DataType_Bool);
        TEST_VALID_EXPRESSION("1 == 1 == 1", DataType_Bool);  // end when encounter second '=='
        TEST_VALID_EXPRESSION("1 > 1.0 ", DataType_Bool);
        TEST_VALID_EXPRESSION("1.7 < b ", DataType_Bool);
        TEST_VALID_EXPRESSION("!bl!", DataType_Bool);
        TEST_VALID_EXPRESSION("(y!) != (-a) *2", DataType_Bool);
    }

    suite("Test valid function expressions") {
        TEST_VALID_EXPRESSION("fn(x: 1, y: 1.1)", DataType_MaybeDouble);
        TEST_VALID_EXPRESSION("1 + fn(x: a + 2 / 3, y: b)! * 2", DataType_Double);
        TEST_VALID_EXPRESSION("1 + many(int: a, b, y, 5.5) * 2", DataType_Double);
        TEST_VALID_EXPRESSION("x() != x() * 2", DataType_Bool);
        TEST_VALID_EXPRESSION("str(name, name) + \"String\"", DataType_String);
        TEST_VALID_EXPRESSION("x()", DataType_Int);
        TEST_VALID_EXPRESSION("n()", DataType_Undefined);
        TEST_VALID_EXPRESSION("one(x: 12)", DataType_Int);
        TEST_VALID_EXPRESSION("write(12, 1, 2, 4)", DataType_Undefined);
    }

    suite("Test valid nil coalescing") {
        TEST_VALID_EXPRESSION("y ?? 0", DataType_Int);
        TEST_VALID_EXPRESSION("(y ?? 0)", DataType_Int);
        TEST_VALID_EXPRESSION("many(int: a, b, y ?? 12, nil)", DataType_Double);
        TEST_VALID_EXPRESSION("fn(x:1, y:2.1) ?? 4.5 ?? 0.5", DataType_Double);
        TEST_VALID_EXPRESSION("y ?? 1+ 2 + a - (-6)", DataType_Int);
        TEST_VALID_EXPRESSION("(s ?? \" \") + s!", DataType_String);
    }

    suite("Test valid nested expression") {
        TEST_VALID_EXPRESSION("2 * fn(x: 8 - 2 * (-14.1) / a, y: fn(x:y!, y:4.5) ?? b)!", DataType_Double);
        TEST_VALID_EXPRESSION(
            "((3.14 * (5.0 + 2.7)) / (2.0 - (-3.5))) + (fn(x: 8, y: (2 * (-14.1))) ?? fn(x: 3, y: 4.5)!)",
            DataType_Double);
        TEST_VALID_EXPRESSION("\"fds\" + str(\"asdf\", \"dsf\") != \"fd\"", DataType_Bool);
    }

    set_print_errors(false);

    suite("Test invalid arithmetic expressions") {
        TEST_INVALID_EXPRESSION("+1", Error_Syntax);
        TEST_INVALID_EXPRESSION("(-1+1", Error_Syntax);
        TEST_INVALID_EXPRESSION(" 1+, a", Error_Syntax);
        TEST_INVALID_EXPRESSION(" (1, a)", Error_Syntax);
        TEST_INVALID_EXPRESSION(" 1!", Error_Operation);
        TEST_INVALID_EXPRESSION("a + b", Error_Operation);
        TEST_INVALID_EXPRESSION("1 + nil", Error_UnknownType);
        TEST_INVALID_EXPRESSION("true + true", Error_Operation);
    }

    suite("Test invalid logic expressions") {
        TEST_INVALID_EXPRESSION("1 <> 2", Error_Syntax);
        TEST_INVALID_EXPRESSION("(1 < !2)", Error_Operation);
        TEST_INVALID_EXPRESSION("(a != b)", Error_Operation);
        TEST_INVALID_EXPRESSION("(true != 45)", Error_Operation);
        TEST_INVALID_EXPRESSION("(nil != b)", Error_UnknownType);
        TEST_INVALID_EXPRESSION("!nil", Error_UnknownType);
        TEST_INVALID_EXPRESSION("!nil!", Error_UnknownType);
        TEST_INVALID_EXPRESSION("y ?? !!false", Error_Syntax);
        TEST_INVALID_EXPRESSION("true < false", Error_Operation);
        TEST_INVALID_EXPRESSION("n() == nil", Error_UnknownType);
        TEST_INVALID_EXPRESSION("nil == nil", Error_UnknownType);
    }

    suite("Test invalid nil coaliscing expressions") {
        TEST_INVALID_EXPRESSION("1 ?? nil", Error_Operation);
        TEST_INVALID_EXPRESSION("y ?? a ?? true", Error_Operation);
        TEST_INVALID_EXPRESSION("y ?? b", Error_Operation);
        TEST_INVALID_EXPRESSION("(bl ?? 4.4) ?? 4", Error_Operation);
    }

    suite("Test invalid function expressions") {
        TEST_INVALID_EXPRESSION("fn(1, 1.5)", Error_TypeMismatched);
        TEST_INVALID_EXPRESSION("fn(x: 1)", Error_TypeMismatched);
        TEST_INVALID_EXPRESSION("fn(x: 1, y: 14, z: 14)", Error_TypeMismatched);
        TEST_INVALID_EXPRESSION("x(nil)", Error_TypeMismatched);
        TEST_INVALID_EXPRESSION("many(int: 1, 1.5, nil, x: nil)", Error_TypeMismatched);
        TEST_INVALID_EXPRESSION("str(1, 1.5)", Error_TypeMismatched);
        TEST_INVALID_EXPRESSION("str(\"1\")", Error_TypeMismatched);
    }

    suite("Test undefined functions and variables") {
        TEST_INVALID_EXPRESSION("u", Error_UndefinedVariable);
        TEST_INVALID_EXPRESSION("a - if", Error_Syntax);
        TEST_INVALID_EXPRESSION("a()", Error_UndefinedFunction);
        TEST_INVALID_EXPRESSION("45()", Error_Syntax);
        TEST_INVALID_EXPRESSION("strv()", Error_UndefinedFunction);
    }

    code_buf_free(&buf);
    parser_free();
    return 0;
}
