/**
 * @brief Implementation of function collect from rec_parser.h
 * @file rec_parser_collect.c
 */
#include "rec_parser.h"
#include "symtable.h"
#include "error.h"
#include "color.h"
#include "to_string.h"
#include <string.h>

// Check if g_parser.token.type is `tok` and if not, set syntax_err with given msg format.
#define bCHECK_TOKEN(tok, ...) \
    if (g_parser.token.type != tok) { \
        syntax_err(__VA_ARGS__); \
        _err = 1;   \
        break; \
    } \
    parser_next_token()

// Call given rule function with parameters and return false if the rule fails.
#define bCALL_RULEp(rule, ...) if (!rule(__VA_ARGS__)) { _err = 1; break;}

#define TRY_FINAL(try, final) do { int _err = 0; do try while(0); if (_err) final } while(0)

#define ERR_BREAK { _err = 1; break; }

bool col_rule_funcReturnType(FunctionSymbol* func);
bool col_rule_params(FunctionSymbol* func);
bool col_rule_params_n(FunctionSymbol* func);
bool col_handle_func_statement();

bool rec_parser_collect() {
    do {
        parser_next_token();
        if (g_parser.token.type == Token_Func) {
            parser_next_token();
            if (!col_handle_func_statement())
                return false;

            // Additionally check that there is a '{' as a start of function definition.
            // We do this, so that during second phase we can just skip to this token,
            // knowing there it is there and we don't iterate to EOF on wrong syntax.
            CHECK_TOKEN(Token_BracketLeft, "Unexpected token `" COL_Y("%s") "` after function declaration. Expected `" COL_C("{") "`.",
                    token_to_string(&g_parser.token));
        }
    } while (g_parser.token.type != Token_EOF);
    return true;
}

bool col_handle_func_statement() {
    TokenAttribute attr = g_parser.token.attribute;
    CHECK_TOKEN(Token_Identifier, "Unexpected token `%s` after the `func` keyword. Expected name of the function.", TOK_STR);
    const char* func_name = attr.data.value.string.data;

    FunctionSymbol func;
    function_symbol_init(&func);

    // Check if function already exists
    if (symtable_get_function(symstack_top(), func_name) != NULL) {
        undef_fun_err("Redefinition of function `" COL_Y("%s") "`.", func_name);
        return false;
    }

    TRY_FINAL({
        bCHECK_TOKEN(Token_ParenLeft, "Unexpected token `%s` after the function name. Expected `(`.", TOK_STR);
        bCALL_RULEp(col_rule_params, &func);
        bCHECK_TOKEN(Token_ParenRight, "Unexpected token `%s` after the function parameters. Expected `)`.", TOK_STR);
        bCALL_RULEp(col_rule_funcReturnType, &func);
    }, {
        function_symbol_free(&func);
        return false;
    });

    if (!symtable_insert_function(symstack_top(), func_name, func)) {
        SET_INT_ERROR(IntError_Runtime, "handle_func_statement: Could not insert function into symtable.");
        return false;
    }

    return true;
}

bool col_rule_params(FunctionSymbol* func) {
    switch (g_parser.token.type) {
        case Token_EOF:
        case Token_ParenRight:
            return true;
        case Token_Identifier: {
            const char* oname = g_parser.token.attribute.data.value.string.data;
            if (strcmp(oname, "_") == 0)
                oname = NULL;
            parser_next_token();

            TokenAttribute attr = g_parser.token.attribute;
            CHECK_TOKEN(Token_Identifier, "Unexpected token `%s` after the parameter name. Expected identifier.", TOK_STR);
            const char* iname = attr.data.value.string.data;

            CHECK_TOKEN(Token_DoubleColon, "Unexpected token `%s` after inner parameter name. Expected `,` or `)`.", TOK_STR);
            attr = g_parser.token.attribute;
            CHECK_TOKEN(Token_DataType, "Unexpected token `%s`. Expected `DataType`.", TOK_STR);

            DataType param_type = attr.data_type;

            // Check if given parameter already exists.
            int has_param;
            if ((has_param = funciton_symbol_has_param(func, oname, iname)) != 0) {
                print_error(&g_parser.token, Error_Semantic, "Semantic",
                            "Conflicting names for parameter `%s%s" PRINT_RESET " %s%s" PRINT_RESET " : %s`.",
                            has_param == 1 ? PRINT_Y : PRINT_W, oname,
                            has_param == 2 ? PRINT_Y : PRINT_W, iname,
                            datatype_to_string(param_type));
                return false;
            }

            // Insert the parameter into Function symbol
            if (!function_symbol_emplace_param(func, param_type, oname, iname))
                return false;

            CALL_RULEp(col_rule_params_n, func);
           } return true;
        default:
            break;
    }
    syntax_err("Unexpected token `%s` in function parameters.", TOK_STR);
    return false;
}

bool col_rule_params_n(FunctionSymbol* func) {
    switch (g_parser.token.type) {
        case Token_EOF:
        case Token_ParenRight:
            return true;
        case Token_Comma:
            parser_next_token();
            return col_rule_params(func);
        default:
            syntax_err("Unexpected token `%s`.", TOK_STR);
            return false;
    }
}

bool col_rule_funcReturnType(FunctionSymbol* func) {
    switch (g_parser.token.type) {
        case Token_EOF:
            syntax_err("Unexpected end of file.");
            return true;
        case Token_BracketLeft:
            func->return_value_type = DataType_Undefined;     // Function doesn't return anything.
            return true;
        case Token_ArrowRight:
            parser_next_token();
            TokenAttribute attr = g_parser.token.attribute;
            CHECK_TOKEN(Token_DataType, "Unexpected token `%s` after `->`. Expected `DataType`.", TOK_STR);

            func->return_value_type = attr.data_type;
            return true;
        default:
            syntax_err("Unexpected token `%s` after ')'. Expected `->` or `{`.", TOK_STR);
            return false;
    }
}

