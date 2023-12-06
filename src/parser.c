/**
 * @brief Implementation for the parser.h
 * @file parser.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#include "parser.h"
#include "rec_parser.h"
#include "scanner.h"
#include "codegen.h"
#include "builtin.h"
#include <math.h>
#include <string.h>

Parser g_parser;

void parser_init() {
    symstack_init();
    if (got_error())
        return;
    // Create the symbol table used for global variables.
    symstack_push();

    g_parser.scope = Scope_Global;
    code_buf_init(&g_parser.global_code);
    if (got_error()) {
        symstack_free();
        return;
    }
    g_parser.currect_code = &g_parser.global_code;
    g_parser.global_var_counter = 0;
    g_parser.local_var_counter = 0;
}

bool add_builtin_functions() {
    builtin_add_readString();
    builtin_add_readInt();
    builtin_add_readDouble();
    builtin_add_readBool();
    builtin_add_write();
    builtin_add_Int2Double();
    builtin_add_Double2Int();
    builtin_add_length();
    builtin_add_substring();
    builtin_add_ord();
    builtin_add_chr();
    return true;
}

void print_all_func_codes(Node* node) {
    if (!node)
        return;
    if (node->type == NodeType_Function) {
        if (node->value.function.is_used)
            code_buf_print(&node->value.function.code);
    }
    print_all_func_codes(node->left);
    print_all_func_codes(node->right);
}

bool parser_begin(bool output_code) {
    code_buf_set(g_parser.currect_code);

    if (!add_builtin_functions())
        return false;
    if (!rec_parser_collect())
        return false;
    scanner_reset_to_beginning();
    if (!rec_parser_begin())
        return false;

    MASSERT(symstack_size() == 1, "Parsing succeeded but there are local symbol tables remaining on the stack.");
    MASSERT(g_parser.scope == Scope_Global, "huh?");

    // TODO: Check if any variable is uninitialized or undefined in symtable.

    // Output code for global statements
    if (output_code) {
        code_buf_print(&g_parser.global_code);
        print_all_func_codes(symstack_bottom()->root);
    }

    return true;
}

void parser_free() {
    symstack_free();
    code_buf_free(&g_parser.global_code);
    g_parser.currect_code = NULL;
}

void parser_scope_function(FunctionSymbol* func) {
    if (!func) {
        parser_scope_global();
        return;
    }
    g_parser.scope = Scope_Local;
    g_parser.currect_code = &func->code;
    g_parser.local_var_counter = func->param_count;
    code_buf_set(g_parser.currect_code);
}

void parser_scope_global() {
    g_parser.scope = Scope_Global;
    g_parser.currect_code = &g_parser.global_code;
    code_buf_set(g_parser.currect_code);
}

Token* parser_next_token() {
    g_parser.token_ws = scanner_advance();
    g_parser.token = g_parser.token_ws.type == Token_Whitespace
                   ? scanner_advance()
                   : g_parser.token_ws;
    MASSERT(g_parser.token.type != Token_DataType || g_parser.token.attribute.data_type != DataType_Undefined, "Scanner cannot return DataType_Undefined");
    return &g_parser.token;
}

bool parser_tok_is_fun_id() {
    NodeType* ntype = symtable_get_symbol_type(symstack_top(), g_parser.token.attribute.data.value.string.data);
    if (!ntype)
        return false;
    return g_parser.token.type == Token_Identifier && *ntype == NodeType_Function;
}

// Insert `name%index` into string `str`
void create_var_name(String* str, const char* name, int index) {
    string_clear(str);
    int index_chars = index != 0 ? (int)((ceil(log10(index))+1)) : 1;
    size_t name_len = strlen(name);
    string_reserve(str, name_len + index_chars + 2);    // + 2 <--- % a \0
    string_concat_c_str(str, name);
    sprintf(str->data + name_len, "%%%i", index);
    str->length = name_len + index_chars + 1;
    str->data[str->length] = '\0';
    MASSERT(str->length <= str->capacity, "Wrong int to string conversion.");
}

void parser_parameter_code_infos(FunctionSymbol* func) {
    for (int i = 0; i < func->param_count; i++) {
        FunctionParameter* param = func->params + i;
        create_var_name(&param->code_name, param->iname.data, i);
    }
}

void parser_variable_code_info(VariableSymbol* var, const char* name) {
    if (g_parser.scope == Scope_Global) {
        create_var_name(&var->code_name, name, g_parser.global_var_counter++);
        var->code_frame = Frame_Global;
    } else {
        create_var_name(&var->code_name, name, g_parser.local_var_counter++);
        var->code_frame = Frame_Local;
    }
}

void parser_function_code_info(FunctionSymbol* func, const char* name) {
    string_clear(&func->code_name);
    int len = strlen("func%") + strlen(name) + 2;
    string_reserve(&func->code_name, len);
    func->code_name.length = len;
    MASSERT(func->code_name.length <= func->code_name.capacity, "");
    sprintf(func->code_name.data, "func%%%s", name);
    func->code_name.data[len - 1] = '\0';

    parser_parameter_code_infos(func);
}

