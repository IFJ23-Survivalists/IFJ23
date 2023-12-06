/**
 * @file test/codegen.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/12/2023
 * @brief Tester for codegen.h
 */


#include "../codegen.h"
#include <string.h>
#include "test.h"

int main() {
    atexit(summary);

    extern CodeBuf *g_code_buf;

    CodeBuf buf1;
    CodeBuf buf2;
    CodeBuf buf3;

    suite("code_buf_init") {
        code_buf_init(&buf1);
        code_buf_init(&buf2);
        code_buf_init(&buf3);

        test(buf1.size == 0);
        test(buf2.size == 0);
        test(buf3.size == 0);
    }

    suite("code_buf_set") {
        code_buf_set(&buf1);
        test(g_code_buf == &buf1);
        code_generation(Instruction_Start, NULL, NULL, NULL);
        code_generation(Instruction_CreateFrame, NULL, NULL, NULL);
        code_generation(Instruction_PushFrame, NULL, NULL, NULL);

        code_buf_set(&buf2);
        test(g_code_buf == &buf2);
        code_generation(Instruction_CreateFrame, NULL, NULL, NULL);
        code_generation(Instruction_PushFrame, NULL, NULL, NULL);

        code_buf_set(&buf3);
        test(g_code_buf == &buf3);
        code_generation(Instruction_PopFrame, NULL, NULL, NULL);

        code_buf_set(&buf1);
        test(g_code_buf->size == 3);
        test(strcmp(g_code_buf->buf[0].code.data, ".IFJcode23") == 0);
        test(strcmp(g_code_buf->buf[1].code.data, "CREATEFRAME") == 0);
        test(strcmp(g_code_buf->buf[2].code.data, "PUSHFRAME") == 0);

        code_buf_set(&buf2);
        test(g_code_buf->size == 2);
        test(strcmp(g_code_buf->buf[0].code.data, "CREATEFRAME") == 0);
        test(strcmp(g_code_buf->buf[1].code.data, "PUSHFRAME") == 0);

        code_buf_set(&buf3);
        test(g_code_buf->size == 1);
        test(strcmp(g_code_buf->buf[0].code.data, "POPFRAME") == 0);
    }

    suite("code_buf_print_to_string") {
        code_buf_free(g_code_buf);

        code_generation(Instruction_Start, NULL, NULL, NULL);
        code_generation(Instruction_CreateFrame, NULL, NULL, NULL);
        code_generation(Instruction_PushFrame, NULL, NULL, NULL);
        code_generation(Instruction_CreateFrame, NULL, NULL, NULL);
        code_generation(Instruction_PushFrame, NULL, NULL, NULL);
        code_generation(Instruction_PopFrame, NULL, NULL, NULL);

        String inst = code_buf_print_to_string(g_code_buf);

        char *expected = ".IFJcode23\nCREATEFRAME\nPUSHFRAME\nCREATEFRAME\nPUSHFRAME\nPOPFRAME\n";
        test(strcmp(inst.data, expected) == 0);
        string_free(&inst);
    }

    suite("code_buf_free") {
        code_buf_free(&buf1);
        code_buf_free(&buf2);
        code_buf_free(&buf3);

        test(!buf1.size);
        test(!buf1.capacity);
        test(!buf1.buf);
        test(!buf2.size);
        test(!buf2.capacity);
        test(!buf2.buf);
        test(!buf3.size);
        test(!buf3.capacity);
        test(!buf3.buf);
    }

    code_buf_set(&buf1);

    suite("Instructions: (0 operand)") {
        code_generation(Instruction_Start, NULL, NULL, NULL);
        code_generation(Instruction_CreateFrame, NULL, NULL, NULL);
        code_generation(Instruction_PushFrame, NULL, NULL, NULL);
        code_generation(Instruction_PopFrame, NULL, NULL, NULL);
        code_generation(Instruction_Return, NULL, NULL, NULL);
        code_generation(Instruction_Clears, NULL, NULL, NULL);
        code_generation(Instruction_Adds, NULL, NULL, NULL);
        code_generation(Instruction_Subs, NULL, NULL, NULL);
        code_generation(Instruction_Muls, NULL, NULL, NULL);
        code_generation(Instruction_Divs, NULL, NULL, NULL);
        code_generation(Instruction_Idivs, NULL, NULL, NULL);
        code_generation(Instruction_Lts, NULL, NULL, NULL);
        code_generation(Instruction_Gts, NULL, NULL, NULL);
        code_generation(Instruction_Eqs, NULL, NULL, NULL);
        code_generation(Instruction_Ands, NULL, NULL, NULL);
        code_generation(Instruction_Ors, NULL, NULL, NULL);
        code_generation(Instruction_Nots, NULL, NULL, NULL);
        code_generation(Instruction_Int2Floats, NULL, NULL, NULL);
        code_generation(Instruction_Float2Ints, NULL, NULL, NULL);
        code_generation(Instruction_Int2Chars, NULL, NULL, NULL);
        code_generation(Instruction_Stri2Ints, NULL, NULL, NULL);
        code_generation(Instruction_Break, NULL, NULL, NULL);

        test(strcmp(buf1.buf[0].code.data, ".IFJcode23") == 0);
        test(strcmp(buf1.buf[1].code.data, "CREATEFRAME") == 0);
        test(strcmp(buf1.buf[2].code.data, "PUSHFRAME") == 0);
        test(strcmp(buf1.buf[3].code.data, "POPFRAME") == 0);
        test(strcmp(buf1.buf[4].code.data, "RETURN") == 0);
        test(strcmp(buf1.buf[5].code.data, "CLEARS") == 0);
        test(strcmp(buf1.buf[6].code.data, "ADDS") == 0);
        test(strcmp(buf1.buf[7].code.data, "SUBS") == 0);
        test(strcmp(buf1.buf[8].code.data, "MULS") == 0);
        test(strcmp(buf1.buf[9].code.data, "DIVS") == 0);
        test(strcmp(buf1.buf[10].code.data, "IDIVS") == 0);
        test(strcmp(buf1.buf[11].code.data, "LTS") == 0);
        test(strcmp(buf1.buf[12].code.data, "GTS") == 0);
        test(strcmp(buf1.buf[13].code.data, "EQS") == 0);
        test(strcmp(buf1.buf[14].code.data, "ANDS") == 0);
        test(strcmp(buf1.buf[15].code.data, "ORS") == 0);
        test(strcmp(buf1.buf[16].code.data, "NOTS") == 0);
        test(strcmp(buf1.buf[17].code.data, "INT2FLOATS") == 0);
        test(strcmp(buf1.buf[18].code.data, "FLOAT2INTS") == 0);
        test(strcmp(buf1.buf[19].code.data, "INT2CHARS") == 0);
        test(strcmp(buf1.buf[20].code.data, "STRI2INTS") == 0);
        test(strcmp(buf1.buf[21].code.data, "BREAK") == 0);

        code_buf_free(&buf1);
    }

    suite("Instructions: symb") {
        Operand symb;

        symb.symbol.type = SymbolType_Variable;
        symb.symbol.variable.frame = Frame_Local;
        symb.symbol.variable.name = "test_sym";

        code_generation(Instruction_Pushs, &symb, NULL, NULL);
        code_generation(Instruction_Write, &symb, NULL, NULL);
        code_generation(Instruction_Exit, &symb, NULL, NULL);
        code_generation(Instruction_DebugPrint, &symb, NULL, NULL);

        test(strcmp(buf1.buf[0].code.data, "PUSHS LF@test_sym") == 0);
        test(strcmp(buf1.buf[1].code.data, "WRITE LF@test_sym") == 0);
        test(strcmp(buf1.buf[2].code.data, "EXIT LF@test_sym") == 0);
        test(strcmp(buf1.buf[3].code.data, "DPRINT LF@test_sym") == 0);

        code_buf_free(&buf1);
    }

    suite("Instructions: var") {
        Operand var;
        var.variable.frame = Frame_Global;
        var.variable.name = "test_var";

        code_generation(Instruction_DefVar, &var, NULL, NULL);
        code_generation(Instruction_Pops, &var, NULL, NULL);

        test(strcmp(buf1.buf[0].code.data, "DEFVAR GF@test_var") == 0);
        test(strcmp(buf1.buf[1].code.data, "POPS GF@test_var") == 0);

        code_buf_free(&buf1);
    }

    suite("Instructions: var type") {
        Operand var;
        Operand type;

        var.variable.frame = Frame_Temporary;
        var.variable.name = "test_var";

        type.data_type = DataType_Int;
        code_generation(Instruction_Read, &var, &type, NULL);
        type.data_type = DataType_Double;
        code_generation(Instruction_Read, &var, &type, NULL);
        type.data_type = DataType_String;
        code_generation(Instruction_Read, &var, &type, NULL);
        type.data_type = DataType_Bool;
        code_generation(Instruction_Read, &var, &type, NULL);

        test(strcmp(buf1.buf[0].code.data, "READ TF@test_var int") == 0);
        test(strcmp(buf1.buf[1].code.data, "READ TF@test_var float") == 0);
        test(strcmp(buf1.buf[2].code.data, "READ TF@test_var string") == 0);
        test(strcmp(buf1.buf[3].code.data, "READ TF@test_var bool") == 0);

        code_buf_free(&buf1);
    }

    suite("Instructions: var, symb") {
        Operand var;
        Operand symb;

        var.variable.frame = Frame_Global;
        var.variable.name = "test_var";
        symb.symbol.type = SymbolType_Constant;
        symb.symbol.constant.value.number = -87842;
        symb.symbol.constant.type = DataType_Int;
        symb.symbol.constant.is_nil = false;

        code_generation(Instruction_Move, &var, &symb, NULL);
        code_generation(Instruction_Int2Float, &var, &symb, NULL);
        code_generation(Instruction_Float2Int, &var, &symb, NULL);
        code_generation(Instruction_Int2Char, &var, &symb, NULL);
        code_generation(Instruction_Stri2Int, &var, &symb, NULL);
        code_generation(Instruction_Strlen, &var, &symb, NULL);
        code_generation(Instruction_Type, &var, &symb, NULL);

        test(strcmp(buf1.buf[0].code.data, "MOVE GF@test_var int@-87842") == 0);
        test(strcmp(buf1.buf[1].code.data, "INT2FLOAT GF@test_var int@-87842") == 0);
        test(strcmp(buf1.buf[2].code.data, "FLOAT2INT GF@test_var int@-87842") == 0);
        test(strcmp(buf1.buf[3].code.data, "INT2CHAR GF@test_var int@-87842") == 0);
        test(strcmp(buf1.buf[4].code.data, "STRI2INT GF@test_var int@-87842") == 0);
        test(strcmp(buf1.buf[5].code.data, "STRLEN GF@test_var int@-87842") == 0);
        test(strcmp(buf1.buf[6].code.data, "TYPE GF@test_var int@-87842") == 0);

        code_buf_free(&buf1);
    }

    suite("Instructions: var, symb, symb") {
        Operand var;
        Operand symb1;
        Operand symb2;

        var.variable.frame = Frame_Global;
        var.variable.name = "test_var";
        symb1.symbol.type = SymbolType_Constant;
        symb1.symbol.constant.value.is_true = true;
        symb1.symbol.constant.type = DataType_Bool;
        symb1.symbol.constant.is_nil = false;
        symb2.symbol.type = SymbolType_Variable;
        symb2.symbol.variable.frame = Frame_Temporary;
        symb2.symbol.variable.name = "test_sym";

        code_generation(Instruction_Add, &var, &symb1, &symb2);
        code_generation(Instruction_Sub, &var, &symb1, &symb2);
        code_generation(Instruction_Mul, &var, &symb1, &symb2);
        code_generation(Instruction_Div, &var, &symb1, &symb2);
        code_generation(Instruction_Idiv, &var, &symb1, &symb2);
        code_generation(Instruction_Lt, &var, &symb1, &symb2);
        code_generation(Instruction_Gt, &var, &symb1, &symb2);
        code_generation(Instruction_Eq, &var, &symb1, &symb2);
        code_generation(Instruction_And, &var, &symb1, &symb2);
        code_generation(Instruction_Or, &var, &symb1, &symb2);
        code_generation(Instruction_Not, &var, &symb1, &symb2);
        code_generation(Instruction_Concat, &var, &symb1, &symb2);
        code_generation(Instruction_GetChar, &var, &symb1, &symb2);
        code_generation(Instruction_SetChar, &var, &symb1, &symb2);

        test(strcmp(buf1.buf[0].code.data, "ADD GF@test_var bool@true TF@test_sym") == 0);
        test(strcmp(buf1.buf[1].code.data, "SUB GF@test_var bool@true TF@test_sym") == 0);
        test(strcmp(buf1.buf[2].code.data, "MUL GF@test_var bool@true TF@test_sym") == 0);
        test(strcmp(buf1.buf[3].code.data, "DIV GF@test_var bool@true TF@test_sym") == 0);
        test(strcmp(buf1.buf[4].code.data, "IDIV GF@test_var bool@true TF@test_sym") == 0);
        test(strcmp(buf1.buf[5].code.data, "LT GF@test_var bool@true TF@test_sym") == 0);
        test(strcmp(buf1.buf[6].code.data, "GT GF@test_var bool@true TF@test_sym") == 0);
        test(strcmp(buf1.buf[7].code.data, "EQ GF@test_var bool@true TF@test_sym") == 0);
        test(strcmp(buf1.buf[8].code.data, "AND GF@test_var bool@true TF@test_sym") == 0);
        test(strcmp(buf1.buf[9].code.data, "OR GF@test_var bool@true TF@test_sym") == 0);
        test(strcmp(buf1.buf[10].code.data, "NOT GF@test_var bool@true TF@test_sym") == 0);
        test(strcmp(buf1.buf[11].code.data, "CONCAT GF@test_var bool@true TF@test_sym") == 0);
        test(strcmp(buf1.buf[12].code.data, "GETCHAR GF@test_var bool@true TF@test_sym") == 0);
        test(strcmp(buf1.buf[13].code.data, "SETCHAR GF@test_var bool@true TF@test_sym") == 0);

        code_buf_free(&buf1);
    }

    suite("Instructions: label") {
        Operand label;
        label.label = "test_label";

        code_generation(Instruction_Call, &label, NULL, NULL);
        code_generation(Instruction_Label, &label, NULL, NULL);
        code_generation(Instruction_Jump, &label, NULL, NULL);
        code_generation(Instruction_JumpIfEqs, &label, NULL, NULL);
        code_generation(Instruction_JumpIfNeqs, &label, NULL, NULL);

        test(strcmp(buf1.buf[0].code.data, "CALL test_label") == 0);
        test(strcmp(buf1.buf[1].code.data, "LABEL test_label") == 0);
        test(strcmp(buf1.buf[2].code.data, "JUMP test_label") == 0);
        test(strcmp(buf1.buf[3].code.data, "JUMPIFEQS test_label") == 0);
        test(strcmp(buf1.buf[4].code.data, "JUMPIFNEQS test_label") == 0);

        code_buf_free(&buf1);
    }

    suite("Instructions: label, symb, symb") {
        Operand label;
        Operand symb1;
        Operand symb2;

        label.label = "test_label";
        symb1.symbol.type = SymbolType_Variable;
        symb1.symbol.variable.frame = Frame_Local;
        symb1.symbol.variable.name = "test_sym";
        symb2.symbol.type = SymbolType_Constant;
        symb2.symbol.constant.is_nil = false;
        symb2.symbol.constant.type = DataType_String;
        symb2.symbol.constant.value.string = string_from_c_str("test");

        code_generation(Instruction_JumpIfEq, &label, &symb1, &symb2);
        code_generation(Instruction_JumpIfNeq, &label, &symb1, &symb2);

        test(strcmp(buf1.buf[0].code.data, "JUMPIFEQ test_label LF@test_sym string@test") == 0);
        test(strcmp(buf1.buf[1].code.data, "JUMPIFNEQ test_label LF@test_sym string@test") == 0);

        string_free(&symb2.symbol.constant.value.string);
        code_buf_free(&buf1);
    }

    suite("Empty Label") {
        Operand label;
        label.label = "";
        code_generation(Instruction_Label, &label, NULL, NULL);

        test(got_error());
        test(!buf1.size);

        set_error(Error_None);
    }

    suite("Label") {
        Operand label;
        label.label = "retezec s lomitkem \\ a\nnovym#radkem";
        code_generation(Instruction_Label, &label, NULL, NULL);

        char *expected = "LABEL retezec\\032s\\032lomitkem\\032\\092\\032a\\010novym\\035radkem";
        test(strcmp(buf1.buf[0].code.data, expected) == 0);
        code_buf_free(&buf1);
    }

    suite("Symbol") {

        code_buf_free(&buf1);
    }

    suite("Literal: Float") {
        Operand symb;
        symb.symbol.type = SymbolType_Constant;
        symb.symbol.constant.is_nil = false;
        symb.symbol.constant.type = DataType_Double;
        symb.symbol.constant.value.number_double = -1e-10;

        code_generation(Instruction_DebugPrint, &symb, NULL, NULL);
        test(strcmp(buf1.buf[0].code.data, "DPRINT float@-0x1.b7cdfd9d7bdbbp-34") == 0);

        code_buf_free(&buf1);
    }

    suite("Literal: String") {
        Operand str;
        str.symbol.type = SymbolType_Constant;
        str.symbol.constant.type = DataType_String;
        str.symbol.constant.value.string = string_from_c_str("retezec s lomitkem \\ a\nnovym#radkem");
        str.symbol.constant.is_nil = false;
        code_generation(Instruction_DebugPrint, &str, NULL, NULL);

        char *expected = "DPRINT string@retezec\\032s\\032lomitkem\\032\\092\\032a\\010novym\\035radkem";
        test(strcmp(buf1.buf[0].code.data, expected) == 0);
        string_free(&str.symbol.constant.value.string);
        code_buf_free(&buf1);
    }

    suite("Literal: nil") {
        Operand nil;
        nil.symbol.type = SymbolType_Constant;
        nil.symbol.constant.type = DataType_Undefined;
        nil.symbol.constant.value.number = 5;
        nil.symbol.constant.is_nil = true;
        code_generation(Instruction_DebugPrint, &nil, NULL, NULL);

        char *expected = "DPRINT nil@nil";
        test(strcmp(buf1.buf[0].code.data, expected) == 0);
        code_buf_free(&buf1);
    }
    return 0;
}
