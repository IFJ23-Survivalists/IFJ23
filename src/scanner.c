/**
 * @file scanner.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 25/09/2023
 * @brief Implementation for the scanner.h
 */

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>

#include "scanner.h"
#include "error.h"

char *KEYWORD[] = {"if", "else", "let", "var", "while", "func", "return", NULL};
Keyword KEYWORD_TYPE[] = {Keyword_If, Keyword_Else, Keyword_Let, Keyword_Var, Keyword_While, Keyword_Func, Keyword_Return};

char *DATA_TYPE_IDENTIFIER[] = {"Int", "Double", "String", "Nil", NULL};
DataType DATA_TYPE[] = {DataType_Int, DataType_Double, DataType_String, DataType_Nil};
DataType OPTIONAL_DATA_TYPE[] = {DataType_MaybeInt, DataType_MaybeDouble, DataType_MaybeString, DataType_Nil};

void token_free(Token *token) {
    bool is_string = token->type == Token_Identifier
        || (token->type == Token_Data && token->attribute.data.type == DataType_String);

    if (is_string) {
        string_free(&token->attribute.data.value.string);
    }
}

typedef enum {
    State_EOF,

    State_Start,
    State_BracketLeft,
    State_BracketRight,
    State_ParenLeft,
    State_ParenRight,
    State_DoubleColon,
    State_At,
    State_Plus,
    State_Minus,
    State_ArrowRight,
    State_Multiply,
    State_Divide,
    State_EqualSign,
    State_DoubleEqualSign,
    State_LessThan,
    State_LessOrEqual,
    State_MoreThan,
    State_MoreOrEqual,
    State_DoubleQuestionMark,
    State_Whitespace,
    State_Identifier,
    State_MaybeNilType,
    State_Number,
    State_NumberDouble,
    State_NumberExponent,
    State_StringEnd,
    State_BlockCommentStart,
    State_BlockCommentEnd,

    State_LineComment,
    State_QuestionMark,
    State_NumberDoubleStart,
    State_NumberExponentStart,
    State_NumberExponentSign,
    State_StringStart,
    State_LineString,
    State_LineStringEscape,
    State_LineStringEscapeUnicode,
    State_LineStringEscapeHexStart,
    State_LineStringEscapeHex1,
    State_LineStringEscapeHex2,
    State_DoubleQuote,
    State_BlockString,
    State_BlockStringEnd1,
    State_BlockStringEnd2,
    State_BlockStringEnd3,
    State_BlockStringEscape,
    State_BlockStringEscapeUnicode,
    State_BlockStringEscapeHexStart,
    State_BlockStringEscapeHex1,
    State_BlockStringEscapeHex2,
} State;

typedef struct {
    FILE *src;
    State current_state;
    int number;
    int decimalpoint;
    int exponent;
    bool is_number_double;
    bool is_exponent_negative;
    String string;
    int line;
    int position_in_line;
    unsigned comment_block_level;
} Scanner;

Scanner scanner;

void scanner_init(FILE *src) {
    if (!src) {
        eprint("File not found\n");
        set_error(Error_Lexical);
        return;
    }

    scanner.src = src;
    scanner.line = 1;
    scanner.position_in_line = 0;
    scanner.current_state = State_Start;
    string_init(&scanner.string);
}

void scanner_free() {
    if (scanner.src) fclose(scanner.src);
    string_free(&scanner.string);
}

static State step(char ch);

void print_position() {
    eprintf("Line: %d Position: %d>", scanner.line, scanner.position_in_line);
}

double make_number_double(int full, int decimalpart) {
    double res = (double)full;
    double point = decimalpart == 0        ? 0.0
                 : decimalpart < 10        ? (double)decimalpart / 10.0
                 : decimalpart < 100       ? (double)decimalpart / 100.0
                 : decimalpart < 1000      ? (double)decimalpart / 1000.0
                 : decimalpart < 10000     ? (double)decimalpart / 10000.0
                 : decimalpart < 100000    ? (double)decimalpart / 100000.0
                 : decimalpart < 1000000   ? (double)decimalpart / 1000000.0
                 : decimalpart < 10000000  ? (double)decimalpart / 10000000.0
                 : decimalpart < 100000000 ? (double)decimalpart / 100000000.0
                                           : (double)decimalpart / 1000000000.0;
                                                               /// 2147483647 je 2^31-1

    if (point > 1.0) {
        print_position();
        eprintf("%d.%d is too large for a Double\n", full, decimalpart);
    }

    return res + point;
}

void get_current_token(Token *token) {
    switch (scanner.current_state) {
        case State_EOF:
            token->type = Token_EOF;
            break;
        case State_BracketLeft:
            token->type = Token_BracketLeft;
            break;
        case State_BracketRight:
            token->type = Token_BracketRight;
            break;
        case State_ParenLeft:
            token->type = Token_ParenLeft;
            break;
        case State_ParenRight:
            token->type = Token_ParenRight;
            break;
        case State_DoubleColon:
            token->type = Token_DoubleColon;
            break;
        case State_ArrowRight:
            token->type = Token_ArrowRight;
            break;
        case State_EqualSign:
            token->type = Token_Equal;
            break;
        case State_Identifier: {
            bool found = false;

            for (int i = 0; KEYWORD[i]; i++) {
                if (strcmp(scanner.string.data, KEYWORD[i]) == 0) {
                    token->type = Token_Keyword;
                    token->attribute.keyword = KEYWORD_TYPE[i];
                    string_clear(&scanner.string);
                    found = true;
                    break;
                }
            }

            if (found) break;

            for (int i = 0; DATA_TYPE_IDENTIFIER[i]; i++) {
                if (strcmp(scanner.string.data, DATA_TYPE_IDENTIFIER[i]) == 0) {
                    token->type = Token_DataType;
                    token->attribute.data_type = DATA_TYPE[i];
                    string_clear(&scanner.string);
                    found = true;
                    break;
                }
            }

            if (found) break;

            token->type = Token_Identifier;
            token->attribute.data.value.string = string_take(&scanner.string);
            break;
        }

        case State_At:
            token->type = Token_At;
            break;
        case State_Plus:
            token->attribute.op = Operator_Plus;
            token->type = Token_Operator;
            break;

        case State_Minus:
            token->attribute.op = Operator_Minus;
            token->type = Token_Operator;
            break;
        case State_Multiply:
            token->attribute.op = Operator_Multiply;
            token->type = Token_Operator;
            break;
        case State_Divide:
            token->attribute.op = Operator_Divide;
            token->type = Token_Operator;
            break;
        case State_DoubleEqualSign:
            token->attribute.op = Operator_DoubleEqual;
            token->type = Token_Operator;
            break;
        case State_LessThan:
            token->attribute.op = Operator_LessThan;
            token->type = Token_Operator;
            break;
        case State_LessOrEqual:
            token->attribute.op = Operator_LessOrEqual;
            token->type = Token_Operator;
            break;
        case State_MoreThan:
            token->attribute.op = Operator_MoreThan;
            token->type = Token_Operator;
            break;
        case State_MoreOrEqual:
            token->attribute.op = Operator_MoreOrEqual;
            token->type = Token_Operator;
            break;
        case State_DoubleQuestionMark:
            token->attribute.op = Operator_DoubleQuestionMark;
            token->type = Token_Operator;
            break;

        case State_MaybeNilType:
            for (int i = 0; DATA_TYPE_IDENTIFIER[i]; i++) {
                if (strcmp(scanner.string.data, DATA_TYPE_IDENTIFIER[i]) == 0) {
                    token->type = Token_DataType;
                    token->attribute.data_type = OPTIONAL_DATA_TYPE[i];
                    string_clear(&scanner.string);
                    break;
                }
            }

            token->type = Token_DataType;
            break;
        case State_Number:
            token->attribute.data.type = DataType_Int;
            token->attribute.data.value.number = scanner.number;
            token->type = Token_Data;
            break;
        case State_NumberDouble:
            token->attribute.data.type = DataType_Double;
            token->attribute.data.value.number_double = make_number_double(scanner.number, scanner.decimalpoint);
            token->type = Token_Data;
            break;
        case State_NumberExponent: {
            double number = make_number_double(scanner.number, scanner.decimalpoint);

            if (got_error()) {
                break;
            }

            if (scanner.is_exponent_negative) {
                while (scanner.exponent--) {
                    number /= 10;
                }

            } else {
                while (scanner.exponent--) {
                    number *= 10;
                }
            }

            token->attribute.data.type = DataType_Double;
            token->attribute.data.value.number_double = number;
            token->type = Token_Data;
            break;
        }
        case State_StringEnd:
            token->attribute.data.type = DataType_String;
            token->attribute.data.value.string = string_take(&scanner.string);
            token->type = Token_Data;
            break;

        case State_Whitespace:
        case State_BlockCommentEnd:
            token->type = Token_Whitespace;
            break;

        default:
            set_error(Error_Internal);
            print_position();
            eprint("Huh? Something went wrong in the tokenization process!!!\n");
            token->type = Token_EOF;
    }
}

Token scanner_advance() {
    Token token = {0};
    bool got_token = false;

    while (!got_token) {
        int ch = fgetc(scanner.src);

        State next_state = ch != EOF ? step(ch) : EOF;

        if (got_error()) {
            return token; /// panik
        }

        token.line = scanner.line;
        token.position_in_line = scanner.position_in_line;

        if (ch == '\n') {
            scanner.line += 1;
            scanner.position_in_line = 0;
        } else {
            scanner.position_in_line += 1;
        }

        if (next_state == State_Start || next_state == State_EOF) {
            ungetc(ch, scanner.src);
            get_current_token(&token);

            if (got_error()) {
                return token;
            }

            got_token = true;
        }

        scanner.current_state = next_state;
    }

    return token;
}

Token scanner_advance_non_whitespace() {
    Token token;

    do {
        token = scanner_advance();
    } while (token.type == Token_Whitespace);

    return token;
}

static bool is_character(char ch) {
    ch |= (1 << 5); // simple binary trick to make ascii character lowercase
    return ch >= 'a' && ch <= 'z';
}

static int parse_decimal(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }

    return -1;
}

static int parse_hexadecimal(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }

    ch |= (1 << 5);

    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }

    return -1;
}

static State step_start(char ch) {
    if (ch >= '0' && ch <= '9') {
        scanner.number = ch - '0';
        // scanner.decimalpoint = 0;
        // scanner.exponent = 0;
        // scanner.is_exponent_negative = false;
        // scanner.is_number_double = false;
        return State_Number;
    }

    if (ch == '_' || is_character(ch)) {
        // string_clear(&scanner.string);
        string_push(&scanner.string, ch);

        if (got_error()) {
            return State_EOF;
        }

        return State_Identifier;
    }

    switch (ch) {
        case ':': return State_DoubleColon;
        case '}': return State_BracketRight;
        case '{': return State_BracketLeft;
        case ')': return State_ParenRight;
        case '(': return State_ParenLeft;
        case '@': return State_At;
        case '+': return State_Plus;
        case '-': return State_Minus;
        case '*': return State_Multiply;
        case '/': return State_Divide;
        case '=': return State_EqualSign;
        case '<': return State_LessThan;
        case '>': return State_MoreThan;
        case '?': return State_QuestionMark;
        case '"': return State_StringStart;

        case 0x20:
        case 0x0A:
        case 0x0D:
        case 0x09:
        case 0x0B:
        case 0x0C:
        case 0x00:
            return State_Whitespace;
    }

    print_position();
    eprintf("Unsupported character `%c`\n", ch);
    set_error(Error_Lexical);
    return State_EOF;
}

static State step_question_mark(char ch) {
    if (ch != '?') {
        print_position();
        eprintf("Expect `?`, found `%c` instead\n", ch);
        set_error(Error_Lexical);
        return State_EOF;
    }

    return State_DoubleQuestionMark;
}

static State step_divide(char ch) {
    switch (ch) {
        case '*':
            scanner.comment_block_level++;
            return State_BlockCommentStart;
        case '/': return State_LineComment;
        default: return State_Start;
    }
}

static State step_identifier(char ch) {
    if (ch == '_' || is_character(ch) || parse_decimal(ch) != -1) {
        string_push(&scanner.string, ch);

        if (got_error()) {
            return State_EOF;
        }
    }

    bool maybe_nil = ch == '?' && (
           strcmp(scanner.string.data, "Int") == 0
        || strcmp(scanner.string.data, "Double") == 0
        || strcmp(scanner.string.data, "String") == 0
    );

    if (maybe_nil) {
        return State_MaybeNilType;
    }

    return State_Start;
}

static void push_number(int *number, int to_push) {
    bool will_overflow = *number > ((INT_MAX - to_push) / 10);

    if (will_overflow) {
        print_position();
        eprint("Number overflow\n");
        set_error(Error_Lexical);
    }

    *number *= 10;
    *number += to_push;
}

static State step_number(char ch) {
    if (ch == '.') {
        return State_NumberDoubleStart;
    }

    if (ch == 'e' || ch == 'E') {
        return State_NumberExponentStart;
    }

    int num = parse_decimal(ch);

    if (num == -1) {
        return State_Start;
    }

    push_number(&scanner.number, num);

    if (got_error()) {
        return State_EOF;
    }

    return State_Number;
}

static State step_number_double_start(char ch) {
    int num = parse_decimal(ch);

    if (num == -1) {
        print_position();
        eprintf("Expect a decimal number, found `%c` instead\n", ch);
        set_error(Error_Lexical);
        return State_EOF;
    }

    scanner.decimalpoint = num;
    scanner.is_number_double = true;
    return State_NumberDouble;
}

static State step_number_double(char ch) {
    if (ch == 'e' || ch == 'E') {
        return State_NumberExponentStart;
    }

    int num = parse_decimal(ch);

    if (num == -1) {
        return State_Start;
    }

    push_number(&scanner.decimalpoint, num);

    return State_NumberDouble;
}
static State step_number_exponent_start(char ch) {
    if (ch == '+') {
        scanner.is_exponent_negative = false;
        return State_NumberExponentSign;
    }

    if (ch == '-') {
        scanner.is_exponent_negative = true;
        return State_NumberExponentSign;
    }

    int num = parse_decimal(ch);

    if (num == -1) {
        print_position();
        eprintf("Expect a decimal number, found `%c` instead\n", ch);
        set_error(Error_Lexical);
        return State_EOF;
    }

    scanner.exponent = num;
    return State_NumberExponent;


}
static State step_number_exponent_sign(char ch) {
    int num = parse_decimal(ch);

    if (num == -1) {
        print_position();
        eprintf("Expect a decimal number, found `%c` instead\n", ch);
        set_error(Error_Lexical);
        return State_EOF;
    }

    scanner.exponent = num;
    return State_NumberExponent;
}

static State step_number_exponent(char ch) {
    int num = parse_decimal(ch);

    if (num == -1) {
        return State_Start;
    }


    push_number(&scanner.exponent, num);
    return State_NumberExponent;
}

static State step_string_start(char ch) {
    if (ch == '"') {
        return State_DoubleQuote;
    }

    if (ch >= 0x20 && ch != '\\' && ch != '\n') {
        string_push(&scanner.string, ch);

        if (got_error()) {
            return State_EOF;
        }

        return State_LineString;
    }

    if (ch == '\\') {
        return State_LineStringEscape;
    }


    print_position();
    eprint("Special character must be escaped by `\\`\n");
    set_error(Error_Lexical);
    return State_EOF;
}

static State step_line_string(char ch) {
    if (ch == '"') {
        return State_StringEnd;
    }

    if (ch == '\\') {
        return State_LineStringEscape;
    }

    if (ch < 0x20 || ch == '\n') {
        print_position();
        eprint("Special character must be escaped by `\\`\n");
        set_error(Error_Lexical);
        return State_EOF;
    }

    string_push(&scanner.string, ch);

    if (got_error()) {
        return State_EOF;
    }

    return State_LineString;
}

static State step_double_quote(char ch) {
    if (ch == '"') {
        return State_BlockString;
    }

    ungetc(ch, scanner.src);
    return State_StringEnd;
}

static State step_string_escape(char ch, bool is_line_string) {
    char to_push;

    switch (ch) {
        case '\\':
            to_push = '\\';
            break;
        case '"':
            to_push = '"';
            break;
        case 'r':
            to_push = '\r';
            break;
        case 't':
            to_push = '\t';
            break;
        case 'n':
            to_push = '\n';
            break;
        case 'u': return is_line_string ? State_LineStringEscapeUnicode : State_BlockStringEscapeUnicode;
        default:
            print_position();
            eprint("Invalid escape sequence in literal\n");
            set_error(Error_Lexical);
            return State_EOF;
    }

    string_push(&scanner.string, to_push);

    if (got_error()) {
        return State_EOF;
    }

    return is_line_string ? State_LineString : State_BlockString;
}

static State step_string_escape_unicode(char ch, bool is_line_string) {
    if (ch != '{') {
        print_position();
        eprintf("Expect `{`, found `%c` instead\n", ch);
        set_error(Error_Lexical);
        return State_EOF;
    }

    return is_line_string ? State_LineStringEscapeHexStart : State_BlockStringEscapeHexStart;
}

static State step_string_escape_hex_start(char ch, bool is_line_string) {
    int num = parse_hexadecimal(ch);

    if (num == -1) {
        print_position();
        eprintf("Expect a hexadecimal number, found `%c` instead\n", ch);
        set_error(Error_Lexical);
        return State_EOF;
    }

    scanner.number = num;
    return is_line_string ? State_LineStringEscapeHex1 : State_BlockStringEscapeHex1;
}

static State step_string_escape_hex(char ch, int nth, bool is_line_string) {
    if (ch == '}') {
        string_push(&scanner.string, ch);

        if (got_error()) {
            return State_EOF;
        }

        return is_line_string ? State_LineString : State_BlockString;
    }

    if (nth == 2) {
        eprintf("Expect `}`, found `%c` instead\n", ch);
        set_error(Error_Lexical);
        return State_EOF;
    }

    int num = parse_hexadecimal(ch);

    if (num == -1) {
        print_position();
        eprintf("Expect `}` or a hexadecimal number, found `%c` instead\n", ch);
        set_error(Error_Lexical);
        return State_EOF;
    }

    scanner.number *= 16;
    scanner.number += num;
    return is_line_string ? State_LineStringEscapeHex2 : State_BlockStringEscapeHex2;
}

static State step_block_string(char ch) {
    if (ch == '"') {
        return State_BlockStringEnd1;
    }

    if (ch == '\\') {
        return State_BlockStringEscape;
    }

    if (ch < 0x20) {
        print_position();
        eprint("Special character must be escaped by `\\`\n");
        set_error(Error_Lexical);
        return State_EOF;
    }

    string_push(&scanner.string, ch);

    if (got_error()) {
        return State_EOF;
    }

    return State_BlockString;
}

static State step_block_string_end(char ch, int nth) {
    if (nth == 1) {
        int ident = ch == ' ' ? 1
                  : ch == '\t' ? 4
                  : 0;

        if (ident) {
            scanner.number += ident;
            return State_BlockStringEnd1;
        }
    }

    if (ch == '"') {
        switch (nth) {
            case 1: return State_BlockStringEnd2;
            case 2: return State_BlockStringEnd3;
            case 3: {
                string_remove_ident(&scanner.string, scanner.number);

                if (got_error()) {
                    set_error(Error_Lexical);
                    return State_EOF;
                }

                return State_StringEnd;
            }
            default: {
                set_error(Error_Internal);
                print_position();
                eprint("Something when wrong");
                return State_EOF;
            }
        }
    }

    while (scanner.number--) {
        string_push(&scanner.string, ' ');
    }

    while (nth-- > 0) {
        string_push(&scanner.string, '"');

        if (got_error()) {
            return State_EOF;
        }
    }

    return State_BlockString;
}

static State step_multiply(char ch) {
    if (ch == '/') {
        scanner.comment_block_level--;
        return State_BlockCommentEnd;
    }

    return State_Start;
}

static State step(char ch) {
    switch (scanner.current_state) {
        case State_EOF: return State_EOF;

        case State_Start: return step_start(ch);
        case State_QuestionMark: return step_question_mark(ch);
        case State_Divide: return step_divide(ch);
        case State_Identifier: return step_identifier(ch);
        case State_Number: return step_number(ch);
        case State_NumberDoubleStart: return step_number_double_start(ch);
        case State_NumberDouble: return step_number_double(ch);
        case State_NumberExponentStart: return step_number_exponent_start(ch);
        case State_NumberExponentSign: return step_number_exponent_sign(ch);
        case State_NumberExponent: return step_number_exponent(ch);
        case State_StringStart: return step_string_start(ch);
        case State_DoubleQuote: return step_double_quote(ch);
        case State_LineString: return step_line_string(ch);
        case State_LineStringEscape: return step_string_escape(ch, true);
        case State_LineStringEscapeUnicode: return step_string_escape_unicode(ch, true);
        case State_LineStringEscapeHexStart: return step_string_escape_hex_start(ch, true);
        case State_LineStringEscapeHex1: return step_string_escape_hex(ch, 1, true);
        case State_LineStringEscapeHex2: return step_string_escape_hex(ch, 2, true);
        case State_BlockString: return step_block_string(ch);
        case State_BlockStringEnd1: return step_block_string_end(ch, 1);
        case State_BlockStringEnd2: return step_block_string_end(ch, 2);
        case State_BlockStringEnd3: return step_block_string_end(ch, 3);
        case State_BlockStringEscape: return step_string_escape(ch, false);
        case State_BlockStringEscapeUnicode: return step_string_escape_unicode(ch, false);
        case State_BlockStringEscapeHexStart: return step_string_escape_hex_start(ch, false);
        case State_BlockStringEscapeHex1: return step_string_escape_hex(ch, 1, false);
        case State_BlockStringEscapeHex2: return step_string_escape_hex(ch, 2, false);

        case State_Minus: return ch == '>' ? State_ArrowRight : State_Start;
        case State_Multiply: return step_multiply(ch);
        case State_EqualSign: return ch == '=' ? State_DoubleEqualSign : State_Start;
        case State_LessThan: return ch == '=' ? State_LessOrEqual : State_Start;
        case State_MoreThan: return ch == '=' ? State_MoreOrEqual : State_Start;
        case State_LineComment: return ch == '\n' ? State_Start : State_LineComment;

        case State_ParenLeft:
        case State_ParenRight:
        case State_BracketLeft:
        case State_BracketRight:
        case State_DoubleColon:
        case State_At:
        case State_Plus:
        case State_ArrowRight:
        case State_DoubleEqualSign:
        case State_LessOrEqual:
        case State_MoreOrEqual:
        case State_DoubleQuestionMark:
        case State_Whitespace:
        case State_MaybeNilType:
        case State_StringEnd:
        case State_BlockCommentStart:
        case State_BlockCommentEnd:
            return State_Start;
    }

    set_error(Error_Lexical);
    return State_EOF;
}