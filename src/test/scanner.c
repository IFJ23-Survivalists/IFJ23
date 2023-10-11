#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../scanner.h"

#define suite(...) if(tst>0&&prev==err)printf("\r \x1b[32m✓\x1b[0m \n");if(printf("   \x1b[1m" __VA_ARGS__ "\x1b[0m"),prev=err,(once=0),1)
#define test(...)  do{(++tst,err+=!(ok=!!(__VA_ARGS__)));if(!ok){if(!once){printf("\r \x1b[31m✗\x1b[0m \n");once=1;}printf("   \x1b[31m✗\x1b[0m %s:%d → %s\n",__FILE__,__LINE__,#__VA_ARGS__);}}while(0)
static unsigned tst=0,err=0,ok=1,prev=0,once=0;
static void summary(void){if(tst>0&&prev==err)printf("\r \x1b[32m✓\x1b[0m \n");printf("\r  \n\x1b[1m\x1b[33m    %d total", tst);printf("\x1b[1m\x1b[32m   %d passed",tst-err);printf("\x1b[1m\x1b[31m   %d failed\x1b\x1b[0m\n",err);exit(err);}

int main() {
    atexit(summary);
    FILE *file = fopen("test/scanner.swift", "r+");

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

        token = scanner_advance();
        test(token.type == Token_Whitespace);

        token = scanner_advance();
        test(token.type == Token_Whitespace);

        token = scanner_advance();
        test(token.type == Token_Whitespace);

        token = scanner_advance();
        test(token.type == Token_Whitespace);

        token = scanner_advance();
        test(token.type == Token_Whitespace);

        token = scanner_advance();
        test(token.type == Token_Whitespace);
    }

    suite("Test Scanner keyword") {
        token = scanner_advance();
        test(token.type == Token_Keyword);
        test(token.attribute.keyword == Keyword_If);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Keyword);
        test(token.attribute.keyword == Keyword_Let);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Keyword);
        test(token.attribute.keyword == Keyword_Var);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Keyword);
        test(token.attribute.keyword == Keyword_Else);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Keyword);
        test(token.attribute.keyword == Keyword_While);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Keyword);
        test(token.attribute.keyword == Keyword_Func);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Keyword);
        test(token.attribute.keyword == Keyword_Return);

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

    }

    suite("Test Scanner literal") {
        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_Int);
        test(token.attribute.data.value.number == 10);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_Double);
        test(token.attribute.data.value.number_double == 39.1);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_Double);
        test(token.attribute.data.value.number_double == 7.0e8);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_Double);
        test(token.attribute.data.value.number_double == 8.0e-5);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_Double);
        test(token.attribute.data.value.number_double == 3.14e2);

    }

    suite("Test Scanner data type") {
        token = scanner_advance_non_whitespace();
        test(token.type == Token_DataType);
        test(token.attribute.data_type == DataType_Nil);

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
        test(token.attribute.data_type == DataType_MaybeInt);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_DataType);
        test(token.attribute.data_type == DataType_MaybeDouble);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_DataType);
        test(token.attribute.data_type == DataType_MaybeString);

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
        token_free(&token);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Identifier);
        test(strcmp(token.attribute.data.value.string.data, "Other2") == 0);
        token_free(&token);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Identifier);
        test(strcmp(token.attribute.data.value.string.data, "_Should_be") == 0);
        token_free(&token);
    }

    suite("Test Scanner string") {
        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_String);
        test(strcmp(token.attribute.data.value.string.data, "Hello \n") == 0);
        token_free(&token);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_String);
        test(strcmp(token.attribute.data.value.string.data, "    No Indent Strip") == 0);
        token_free(&token);

        token = scanner_advance_non_whitespace();
        test(token.type == Token_Data);
        test(token.attribute.data.type == DataType_String);
        test(strcmp(token.attribute.data.value.string.data, "Ident stripped") == 0);
        token_free(&token);

        token = scanner_advance_non_whitespace();
        test(got_error());
    }

    scanner_free();
}
