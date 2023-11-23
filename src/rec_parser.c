/**
 * @file pred_parser.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#include "rec_parser.h"
#include "error.h"
#include "scanner.h"
#include <stdarg.h>
#include <string.h>

// Current token on the input.
Token g_token;

inline void get_next_token() {
    g_token = scanner_advance();
}

// FIXME: This is not needed after it will be changed in scanner.h
#define Token_EOL Token_Whitespace

bool rule_statementList();
bool rule_statement();
bool rule_params();
bool rule_params_n();
bool rule_returnExpr();
bool rule_ifCondition();
bool rule_else();
bool rule_assignType();
bool rule_assignExpr();

bool rec_parser_begin() {
    get_next_token();
    return rule_statementList();
}

bool rule_statementList() {
    bool res = false;

    switch (g_token.type) {
        case Token_Keyword:
            switch (g_token.attribute.keyword) {        // FIXME: May be moved to TokenType in the future.
                case Keyword_If:
                case Keyword_Let:
                case Keyword_Var:
                case Keyword_While:
                case Keyword_Func:
                case Keyword_Return: {
                    bool stmt_res = rule_statement();
                    get_next_token();
                    bool has_eol = true;
                    get_next_token();
                    return stmt_res && has_eol && rule_statementList();
                } break;
                default:
                    break;
            }
            break;
        case Token_Identifier: {
            bool stmt_res = rule_statement();
            get_next_token();
            bool has_eol = true;
            get_next_token();
            return stmt_res && has_eol && rule_statementList();
        } break;
        case Token_EOF:
        case Token_EOL:
        case Token_BracketRight:
            return true;
        default:
            break;
    }

    return res;
}

// typedef bool RuleFunc();

bool expand_list(const char *fmt, va_list args) {
    (void)fmt;
    (void)args;
    return true;
    // size_t fmt_len = strlen(fmt);
    // for (size_t i = 0; i < fmt_len; i++) {
    //     switch (fmt[i]) {
    //         case 'k': {
    //             RuleFunc* f = va_arg(args, RuleFunc*);
    //         } break;
    //         case 'r':
    //             break;
    //         case 't':
    //             break;
    //         default: break;
    //     }
    // }
}

bool expand(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bool res = expand_list(fmt, args);
    va_end(args);
    return res;
}

bool rule_statement() {
    bool res = false;
    switch (g_token.type) {
        case Token_Keyword:
            switch (g_token.attribute.keyword) {
                case Keyword_If:
                    res = expand("krtrtr", Keyword_If, rule_ifCondition, Token_BracketLeft,
                                 rule_statementList, Token_BracketRight, rule_statementList);
                    break;
                case Keyword_Let:
                    break;
                case Keyword_Var:
                    break;
                case Keyword_While:
                    break;
                case Keyword_Func:
                    break;
                case Keyword_Return:
                    break;
                default:
                    break;
            }
            break;
        case Token_Identifier:
            break;
        default:
            break;
    }
    return res;
}
bool rule_params() {
    bool res = false;
    return res;
}
bool rule_params_n() {
    bool res = false;
    return res;
}
bool rule_returnExpr() {
    bool res = false;
    return res;
}
bool rule_ifCondition() {
    bool res = false;
    return res;
}
bool rule_else() {
    bool res = false;
    return res;
}
bool rule_assignType() {
    bool res = false;
    return res;
}
bool rule_assignExpr() {
    bool res = false;
    return res;
}

