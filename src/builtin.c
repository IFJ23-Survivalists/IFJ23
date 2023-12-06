/**
 * @file builtin.c
 * @author Lukáš Habr
 */
#include "scanner.h"
#include "parser.h"
#include "symtable.h"
#include "builtin.h"

typedef struct param_info {
    DataType dt;
    const char *iname;
    const char *oname;
} ParamInfo;

void builtin_add_function(DataType return_data_type, const char* function_name, char** code, size_t code_size, ParamInfo* params, size_t  params_size) {
    FunctionSymbol func;
    function_symbol_init(&func);
    func.return_value_type = return_data_type;
    for (size_t i = 0; i < params_size; i++) {
        function_symbol_emplace_param(&func, params[i].dt, params[i].oname, params[i].iname);
    }
    parser_function_code_info(&func, function_name);
    code_buf_set(&func.code);

    Operand op;
    op.label = func.code_name.data;
    code_generation(Instruction_Label, &op, NULL, NULL);
    code_generation(Instruction_PushFrame, NULL, NULL, NULL);
    code_generation_raw("DEFVAR LF@ret");

    // Code generation
    code_buf_set(&func.code);       // Generate code into this function.
    for (size_t i = 0; i < code_size; i++) {
        const char* stmt = code[i];
        code_generation_raw("%s", stmt);
    }

    code_generation(Instruction_PopFrame, NULL, NULL, NULL);
    code_generation(Instruction_Return, NULL, NULL, NULL);
    code_buf_set(&g_parser.global_code);
    symtable_insert_function(symstack_bottom(), function_name, func);
}

void builtin_add_readString() {
    char *code[] = {
        "READ LF@ret string"
    };
    builtin_add_function(DataType_MaybeString, "readString", code, 1, NULL, 0);
}

void builtin_add_readInt() {
    char *code[] = {
        "READ LF@ret int"
    };
    builtin_add_function(DataType_MaybeInt, "readInt", code, 1, NULL, 0);
}
void builtin_add_readDouble() {
    char *code[] = {
        "READ LF@ret float"
    };
    builtin_add_function(DataType_MaybeDouble, "readDouble", code, 1, NULL, 0);
}
void builtin_add_readBool() {
    char *code[] = {
        "READ LF@ret bool"
    };
    builtin_add_function(DataType_MaybeBool, "readBool", code, 1, NULL, 0);
}

void builtin_add_write() {
    FunctionSymbol func;
    function_symbol_init(&func);
    symtable_insert_function(symstack_bottom(), "write", func);
}

void builtin_add_Int2Double() {
    char *code[] = {
        "INT2FLOAT LF@ret LF@term%0"
    };
    ParamInfo params[] = {
        (ParamInfo){ .dt = DataType_Int, .oname = NULL, .iname = "term" },
    };
    builtin_add_function(DataType_Double, "Int2Double", code, 1, params, 1);
}
void builtin_add_Double2Int() {
    char *code[] = {
        "FLOAT2INT LF@ret LF@term%0"
    };
    ParamInfo params[] = {
        (ParamInfo){ .dt = DataType_Double, .oname = NULL, .iname = "term" },
    };
    builtin_add_function(DataType_Int, "Double2Int", code, 1, params, 1);
}
void builtin_add_length() {
    char *code[] = {
        "STRLEN LF@ret LF@s%0"
    };
    ParamInfo params[] = {
        (ParamInfo){ .dt = DataType_String, .oname = NULL, .iname = "s" },
    };
    builtin_add_function(DataType_Int, "length", code, 1, params, 1);
}
void builtin_add_substring() {
    char* code[] = {
        "DEFVAR LF@tmp",
        "DEFVAR LF@len",
        "STRLEN LF@len LF@s%0",

        "LT LF@tmp LF@i%1 int@0",
        "JUMPIFEQ substring_ret_nil LF@tmp bool@true",

        "LT LF@tmp LF@j%2 int@0",
        "JUMPIFEQ substring_ret_nil LF@tmp bool@true",

        "GT LF@tmp LF@i%1 LF@j%2",
        "JUMPIFEQ substring_ret_nil LF@tmp bool@true",

        "GT LF@tmp LF@i%1 LF@len",
        "JUMPIFEQ substring_ret_nil LF@tmp bool@true",

        "EQ LF@tmp LF@i%1 LF@len",
        "JUMPIFEQ substring_ret_nil LF@tmp bool@true",

        "GT LF@tmp LF@j%2 LF@len",
        "JUMPIFEQ substring_ret_nil LF@tmp bool@true",

        "MOVE LF@ret string@",
        "DEFVAR LF@pos_i",
        "MOVE LF@pos_i int@0",
        "DEFVAR LF@pos_j",
        "MOVE LF@pos_j int@0",
        "DEFVAR LF@char",

        "LABEL substring_i_while_start",
        "LT LF@tmp LF@pos_i LF@i%1",
        "JUMPIFNEQ substring_i_while_end LF@tmp bool@true",
        "ADD LF@pos_i LF@pos_i int@1",
        "JUMP substring_i_while_start",
        "LABEL substring_i_while_end",

        "MOVE LF@pos_j LF@pos_i",
        "LABEL sustring_j_while_start",
        "LT LF@tmp LF@pos_j LF@j%2",
        "JUMPIFNEQ substring_j_while_end LF@tmp bool@true",
        "GETCHAR LF@char LF@s%0 LF@pos_j",
        "CONCAT LF@ret LF@ret LF@char",
        "ADD LF@pos_j LF@pos_j int@1",
        "JUMP sustring_j_while_start",
        "LABEL substring_j_while_end",

        "JUMP substring_end",

        "LABEL substring_ret_nil",
        "MOVE LF@ret nil@nil",
        "LABEL substring_end",
    };
    ParamInfo params[] = {
        (ParamInfo){ .dt = DataType_String, .oname = "of", .iname = "s" },
        (ParamInfo){ .dt = DataType_Int, .oname = "startingAt", .iname = "i" },
        (ParamInfo){ .dt = DataType_Int, .oname = "endingBefore", .iname = "j" },
    };
    builtin_add_function(DataType_MaybeString, "substring", code, sizeof(code) / sizeof(*code), params, 3);
}

void builtin_add_ord() {
    char* code[] = {
        "DEFVAR LF@len",
        "STRLEN LF@len LF@c%0",
        "JUMPIFEQ ord_end_0 LF@len int@0",

        "STRI2INT LF@ret LF@c%0 int@0",
        "JUMP ord_end",

        "LABEL ord_end_0",
        "MOVE LF@ret int@0",
        "LABEL ord_end",
    };
    ParamInfo params[] = {
        (ParamInfo){ .dt = DataType_String, .oname = NULL, .iname = "c" },
    };
    builtin_add_function(DataType_Int, "ord", code, sizeof(code) / sizeof(*code), params, 1);
}

void builtin_add_chr() {
    char* code[] = {
        "INT2CHAR LF@ret LF@i%0"
    };
    ParamInfo params[] = {
        (ParamInfo){ .dt = DataType_Int, .oname = NULL, .iname = "i" },
    };
    builtin_add_function(DataType_String, "chr", code, 1, params, 1);
}

