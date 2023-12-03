/**
 * @file codegen.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/12/2023
 * @brief Implementation for the codegen.h
 */

#include "codegen.h"
#include <string.h>
#include <stdio.h>

CodeBuf *g_code_buf = NULL;

const int c_buf_chunk = 10;

void code_buf_init(CodeBuf *buf) {
    if (!buf) return;

    buf->size = 0;
    buf->capacity = c_buf_chunk;
    buf->buf = malloc(sizeof(GeneratedInstruction) * c_buf_chunk);

    if (!buf->buf) {
        set_error(Error_Internal);
        eprint("Out Of Memory");
        return;
    }
}

void code_buf_set(CodeBuf *buf) {
    g_code_buf = buf;
}

void code_buf_print(CodeBuf *buf) {
    for (size_t i = 0; i < buf->size; i++) {
        printf("%s\n", buf->buf[i].code.data);
    }
}

void code_buf_free(CodeBuf *buf) {
    if (!buf) return;

    for (size_t i = 0; i < buf->size; i++) {
        string_free(&buf->buf[i].code);
    }

    free(buf->buf);

    buf->buf = NULL;
    buf->size = 0;
}

void code_buf_push(CodeBuf *buf, String instruction_str) {
    if (buf->size >= buf->capacity) {
        size_t new_capacity = buf->capacity + c_buf_chunk;
        GeneratedInstruction *tmp = realloc(buf->buf, sizeof(GeneratedInstruction) * new_capacity);

        if (!tmp) {
            set_error(Error_Internal);
            eprint("Out Of Memory");
            return;
        }

        buf->buf = tmp;
        buf->capacity = new_capacity;
    }

    GeneratedInstruction generated_inst;
    generated_inst.code = instruction_str;
    g_code_buf->buf[g_code_buf->size++] = generated_inst;
}

void string_insert_constant(String *str, Data data) {
    if (data.is_nil) {
        string_concat_c_str(str, "nil@nil");
        return;
    }

    switch (data.type) {
        case DataType_Int: {
            // int@ is 4 char, maximum value of an interger is 19 + 1 for the sign,
            // also 1 for the \0
            char ch[25] = "int@";
            snprintf(ch + 4, 21, "%d", data.value.number);
            string_concat_c_str(str, ch);
            break;
        }

        case DataType_Double: {
            int len = snprintf(NULL, 0, "%a", data.value.number_double);

            string_reserve(str, str->length + strlen("float@") + len + 1);

            if (got_error()) {
                return;
            }

            string_concat_c_str(str, "float@");
            snprintf(str->data + str->length, len + 1, "%a", data.value.number_double);
            str->length += len;

            break;
        }

        case DataType_String: {
            string_reserve(str, str->length + data.value.string.length);

             if (got_error()) {
                return;
             }

            for (int i = 0; data.value.string.data[i]; i++) {
                char ch = data.value.string.data[i];
                if (ch == 35 || ch == 92 || ch <= 32) {
                    char conv[6] = "\\";
                    snprintf(conv + 1, 5, "%03d", ch);
                    string_concat_c_str(str, conv);

                } else {
                    string_push(str, ch);
                }

                if (got_error()) {
                    return;
                }
            }

            break;
        }

        case DataType_Bool: {
            char *s = data.value.is_true ? "bool@true" : "bool@false";
            string_concat_c_str(str, s);
            break;
        }

        default:
            set_error(Error_Internal);
            break;
    }
}

void code_generation(Instruction inst, Operand *op1, Operand *op2, Operand *op3) {
    if (!g_code_buf) {
        set_error(Error_Internal);
        eprint("code_generation: CodeBuf is not initialized");
        return;
    }

    String instruction_str;
    string_init(&instruction_str);

    if (got_error()) {
        return;
    }

    #define push_str(s) \
        do { \
            string_concat_c_str(&instruction_str, s); \
            if (got_error()) return; \
        } while(0)

    #define push_var(var) \
        do { \
            switch (var.frame) { \
                case Frame_Global: push_str("GF@"); break; \
                case Frame_Local: push_str("LF@"); break; \
                case Frame_Temporary: push_str("TF@"); break; \
            } \
            push_str(var.name); \
        } while(0)

    #define push_symb(symb) \
        do { \
            switch (symb.type) { \
                case SymbolType_Variable: \
                    push_var(symb.variable); \
                    break; \
                case SymbolType_Constant: \
                    push_str(" "); \
                    string_insert_constant(&instruction_str, symb.constant); \
                    break; \
            } \
        } while(0)

    #define push_label(label) push_str(" "); push_str(label)

    #define push_to_buf() \
        do { \
            code_buf_push(g_code_buf, instruction_str); \
            if (got_error()) return; \
        } while(0)

    #define push_instruction(inst) \
        do { \
            push_str(inst); \
            push_to_buf(); \
        } while(0)

    #define push_instruction_var(inst) \
        do { \
            push_str(inst); \
            push_var(op1->variable); \
            push_to_buf(); \
        } while(0)

    #define push_instruction_var_symb(inst) \
        do { \
            push_str(inst); \
            push_var(op1->variable); \
            push_symb(op2->symbol); \
            push_to_buf(); \
        } while(0)

    #define push_instruction_label(inst) \
        do { \
            push_str(inst); \
            push_label(op1->label); \
            push_to_buf(); \
        } while(0)

    #define push_instruction_symb(inst) \
        do { \
            push_str(inst); \
            push_symb(op1->symbol); \
            push_to_buf(); \
        } while(0)

    #define push_instruction_var_symb_symb(inst) \
        do { \
            push_str(inst); \
            push_var(op1->variable); \
            push_symb(op2->symbol); \
            push_symb(op3->symbol); \
            push_to_buf(); \
        } while(0)

    #define push_instruction_var_type(inst) \
        do { \
            push_str(inst); \
            push_label(op1->label); \
            switch (op2->data_type) { \
                case DataType_MaybeInt: \
                case DataType_Int: \
                    push_str(" Int"); \
                    break;\
                case DataType_MaybeDouble: \
                case DataType_Double: \
                    push_str(" Double"); \
                    break; \
                case DataType_MaybeString: \
                case DataType_String: \
                    push_str(" String"); \
                    break; \
                case DataType_MaybeBool: \
                case DataType_Bool: \
                    push_str(" Bool"); \
                    break; \
                case DataType_Undefined: \
                    push_str(" nil"); \
                    break; \
            } \
            push_to_buf(); \
        } while(0)

    #define push_instruction_label_symb_symb(inst) \
        do { \
            push_str(inst); \
            push_label(op1->label); \
            push_symb(op2->symbol); \
            push_symb(op3->symbol); \
            push_to_buf(); \
        } while(0)

    switch (inst) {
        case Instruction_Start: push_instruction(".IFJcode23"); break;
        case Instruction_Move: push_instruction_var_symb("MOVE"); break;
        case Instruction_CreateFrame: push_instruction("CREATEFRAME"); break;
        case Instruction_PushFrame: push_instruction("PUSHFRAME"); break;
        case Instruction_PopFrame: push_instruction("POPFRAME"); break;
        case Instruction_DefVar: push_instruction_var("DEFVAR"); break;
        case Instruction_Call: push_instruction_label("CALL"); break;
        case Instruction_Return: push_instruction("RETURN"); break;
        case Instruction_Pushs: push_instruction_symb("PUSHS"); break;
        case Instruction_Pops: push_instruction_var("POPS"); break;
        case Instruction_Clears: push_instruction("CLEARS"); break;
        case Instruction_Add: push_instruction_var_symb_symb("ADD"); break;
        case Instruction_Sub: push_instruction_var_symb_symb("SUB"); break;
        case Instruction_Mul: push_instruction_var_symb_symb("MUL"); break;
        case Instruction_Div: push_instruction_var_symb_symb("DIV"); break;
        case Instruction_Idiv: push_instruction_var_symb_symb("IDIV"); break;
        case Instruction_Adds: push_instruction("ADD"); break;
        case Instruction_Subs: push_instruction("SUB"); break;
        case Instruction_Muls: push_instruction("MUL"); break;
        case Instruction_Divs: push_instruction("DIV"); break;
        case Instruction_Idivs: push_instruction("IDIV"); break;
        case Instruction_Lt: push_instruction_var_symb_symb("LT"); break;
        case Instruction_Gt: push_instruction_var_symb_symb("GT"); break;
        case Instruction_Eq: push_instruction_var_symb_symb("EQ"); break;
        case Instruction_Lts: push_instruction("LTS"); break;
        case Instruction_Gts: push_instruction("GTS"); break;
        case Instruction_Eqs: push_instruction("EQS"); break;
        case Instruction_And: push_instruction_var_symb_symb("AND"); break;
        case Instruction_Or: push_instruction_var_symb_symb("OR"); break;
        case Instruction_Not: push_instruction_var_symb_symb("NOT"); break;
        case Instruction_Ands: push_instruction("ANDS"); break;
        case Instruction_Ors: push_instruction("ORS"); break;
        case Instruction_Nots: push_instruction("NOTS"); break;
        case Instruction_Int2Float: push_instruction_var_symb("INT2FLOAT"); break;
        case Instruction_FLoat2Int: push_instruction_var_symb("FLOAT2INT"); break;
        case Instruction_Int2Char: push_instruction_var_symb("INT2CHAR"); break;
        case Instruction_Stri2Int: push_instruction_var_symb("STRI2INT"); break;
        case Instruction_Int2Floats: push_instruction("INT2FLOATs"); break;
        case Instruction_FLoat2Ints: push_instruction("FLOAT2INTs"); break;
        case Instruction_Int2Chars: push_instruction("INT2CHARs"); break;
        case Instruction_Stri2Ints: push_instruction("STRI2INTs"); break;
        case Instruction_Read: push_instruction_var_type("READ"); break;
        case Instruction_Write: push_instruction_symb("WRITE"); break;
        case Instruction_Concat: push_instruction_var_symb_symb("CONCAT"); break;
        case Instruction_Strlen: push_instruction_var_symb("STRLEN"); break;
        case Instruction_GetChar: push_instruction_var_symb_symb("GETCHAR"); break;
        case Instruction_SetChar: push_instruction_var_symb_symb("SETCHAR"); break;
        case Instruction_Type: push_instruction_var_symb("TYPE"); break;
        case Instruction_Label: push_instruction_label("LABEL"); break;
        case Instruction_Jump: push_instruction_label("JUMP"); break;
        case Instruction_JumpIfEq: push_instruction_label_symb_symb("JUMPIFEQ"); break;
        case Instruction_JumpIfNeq: push_instruction_label_symb_symb("JUMPIFNEQ"); break;
        case Instruction_JumpIfEqs: push_instruction_label("JUMPIFEQS"); break;
        case Instruction_JumpIfNeqs: push_instruction_label("JUMPIFNEQS"); break;
        case Instruction_Exit: push_instruction_symb("EXIT"); break;
        case Instruction_Break: push_instruction("BREAK"); break;
        case Instruction_DebugPrint: push_instruction_symb("DPRINT"); break;
    }
}
