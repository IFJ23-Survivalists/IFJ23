/*
 * @note Project: Implementace překladače imperativního jazyka IFJ23
 * @file test/scanner.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 10/10/2023
 * @brief Tester for scanner.h
 */

#include "../scanner.h"
#include <string.h>
#include "test.h"

int main() {
    atexit(summary);
    FILE* file = fopen("test/scanner.swift", "r+");

    if (!file) {
        eprint("Unable to read file\n");
        return 99;
    }

    scanner_init(file);

    if (got_error()) {
        return 1;
    }

    Token token;

    suite("Test Scanner Whitespace") {
        token = scanner_advance();
        test(token.type == Token_Whitespace);
        test(token.attribute.has_eol);
    }

    suite("Test Scanner keyword") {
        token = scanner_advance();
        test(token.type == Token_If);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Let);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Var);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Else);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_While);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Func);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Return);
    }

    suite("Test Scanner operator") {
        token = scanner_advance_non_whitespace();
        test(token.type == Token_Operator);
        test(token.attribute.op == Operator_Plus);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Operator);
        test(token.attribute.op == Operator_Minus);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Operator);
        test(token.attribute.op == Operator_Multiply);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Operator);
        test(token.attribute.op == Operator_Divide);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Operator);
        test(token.attribute.op == Operator_DoubleQuestionMark);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Equal);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Operator);
        test(token.attribute.op == Operator_LessOrEqual);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Operator);
        test(token.attribute.op == Operator_DoubleEqual);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Operator);
        test(token.attribute.op == Operator_MoreThan);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Operator);
        test(token.attribute.op == Operator_MoreOrEqual);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Operator);
        test(token.attribute.op == Operator_Negation);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Operator);
        test(token.attribute.op == Operator_NotEqual);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Operator);
        test(token.attribute.op == Operator_Or);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Operator);
        test(token.attribute.op == Operator_And);
    }

    suite("Test Scanner literal") {
        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_Int);
        test(token.attribute.data.value.number == 10);
        test(!token.attribute.data.is_nil);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_Double);
        test(token.attribute.data.value.number_double == 39.1);
        test(!token.attribute.data.is_nil);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_Double);
        test(token.attribute.data.value.number_double == 7.0e8);
        test(!token.attribute.data.is_nil);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_Double);
        test(token.attribute.data.value.number_double == 8.0e-5);
        test(!token.attribute.data.is_nil);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_Double);
        test(token.attribute.data.value.number_double == 3.14e2);
        test(!token.attribute.data.is_nil);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_Bool);
        test(token.attribute.data.value.is_true);
        test(!token.attribute.data.is_nil);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_Bool);
        test(!token.attribute.data.value.is_true);
        test(!token.attribute.data.is_nil);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.is_nil);
    }

    suite("Test Scanner data type") {
        token = scanner_advance_non_whitespace();
        test(token.type == Token_DataType);
        test(token.attribute.data_type == DataType_Int);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_DataType);
        test(token.attribute.data_type == DataType_Double);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_DataType);
        test(token.attribute.data_type == DataType_String);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_DataType);
        test(token.attribute.data_type == DataType_Bool);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_DataType);
        test(token.attribute.data_type == DataType_MaybeInt);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_DataType);
        test(token.attribute.data_type == DataType_MaybeDouble);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_DataType);
        test(token.attribute.data_type == DataType_MaybeString);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_DataType);
        test(token.attribute.data_type == DataType_MaybeBool);
    }

    suite("Test Scanner other token") {
        token = scanner_advance_non_whitespace();
        test(token.type == Token_BracketLeft);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_BracketRight);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_ParenLeft);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_ParenRight);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_DoubleColon);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Comma);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_ArrowRight);
    }

    suite("Test Scanner identifier") {
        token = scanner_advance_non_whitespace();
        test(token.type == Token_Identifier);
        test(strcmp(token.attribute.data.value.string.data, "SomeID") == 0);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Identifier);
        test(strcmp(token.attribute.data.value.string.data, "Other2") == 0);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Identifier);
        test(strcmp(token.attribute.data.value.string.data, "_Should_be") == 0);
    }

    suite("Test Scanner string") {
        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_String);
        test(strcmp(token.attribute.data.value.string.data, "Hello \n") == 0);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_String);
        test(strcmp(token.attribute.data.value.string.data, "    No Indent Strip") == 0);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_String);
        test(strcmp(token.attribute.data.value.string.data, "Ident stripped") == 0);

        token = scanner_advance_non_whitespace();
        test(got_error());
    }

    suite("Test Scanner seek back") {
        scanner_reset_to_beginning();

        token = scanner_advance_non_whitespace();
        test(token.type == Token_If);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Let);
    }

    scanner_free();

    suite("Test Scanner init str") {
        set_error(Error_None);
        scanner_init_str("let a: Bool? = \"Hello World!\"");
        Token token = scanner_advance_non_whitespace();
        test(token.type == Token_Let);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Identifier);
        test(strcmp(token.attribute.data.value.string.data, "a") == 0);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_DoubleColon);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_DataType);
        test(token.attribute.data_type == DataType_MaybeBool);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Equal);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_String);
        test(strcmp(token.attribute.data.value.string.data, "Hello World!") == 0);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_EOF);

        scanner_reset_to_beginning();
        token = scanner_advance_non_whitespace();
        test(token.type == Token_Let);

        scanner_free();
    }
}
