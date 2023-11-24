/**
 * @file pred_parser.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#include <stdarg.h>
#include <string.h>

#include "rec_parser.h"
#include "expr_parser.h"
#include "error.h"
#include "scanner.h"
#include "parser.h"

// Current token on the input.
Token g_token;

inline void get_next_token() {
    g_token = scanner_advance_non_whitespace();
}

inline void get_next_token_ws() {
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

bool check_token(TokenType tok) {
    if (g_token.type == tok) {
        get_next_token();
        return true;
    }
    set_error(Error_Syntax);
    return false;
}

bool rule_statementList() {
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
                    get_next_token_ws();
                    bool has_eol = g_token.type == Token_Whitespace && g_token.attribute.has_eol;
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

    return false;
}

bool rule_statement() {
    switch (g_token.type) {
        case Token_Keyword:
            switch (g_token.attribute.keyword) {
                case Keyword_If:
                    get_next_token();
                    return rule_ifCondition()
                        && check_token(Token_BracketLeft)
                        && rule_statementList()
                        && check_token(Token_BracketRight)
                        && rule_statementList();
                case Keyword_Let:
                    get_next_token();
                    return check_token(Token_Identifier)
                        && rule_assignType()
                        && check_token(Token_Equal)
                        && expr_parser_begin(g_token);
                case Keyword_Var:
                    get_next_token();
                    return check_token(Token_Identifier)
                        && rule_assignType()
                        && rule_assignExpr();
                case Keyword_While:
                    get_next_token();
                    return check_token(Token_ParenLeft)
                        && expr_parser_begin(g_token)
                        && check_token(Token_ParenRight)
                        && check_token(Token_BracketLeft)
                        && rule_statementList()
                        && check_token(Token_BracketRight)
                        && rule_statementList();
                case Keyword_Func:
                    get_next_token();
                    return check_token(Token_Identifier)
                        && check_token(Token_ParenLeft)
                        && rule_params()
                        && check_token(Token_ParenRight)
                        && check_token(Token_ArrowRight)
                        && check_token(Token_DataType)
                        && check_token(Token_BracketLeft)
                        && rule_statementList()
                        && check_token(Token_BracketRight)
                        && rule_statementList();
                case Keyword_Return:
                    get_next_token();
                    return rule_returnExpr();
                default:
                    break;
            }
            break;
        case Token_Identifier:
            // Check if this ID is a function or not. Based on that select the correct rule to use.
            if (symtable_get_function(&g_symtable, g_token.attribute.data.value.string.data) != NULL) {
                return expr_parser_begin(g_token);
            }
            get_next_token();
            return check_token(Token_Equal) && expr_parser_begin(g_token);
        default:
            break;
    }
    return false;
}
bool rule_params() {
    if (g_token.type == Token_ParenRight)
        return true;
    if (g_token.type == Token_Identifier || g_token.type == Token_EOF) {
        get_next_token();
        return check_token(Token_Identifier)
            && check_token(Token_DoubleColon)
            && check_token(Token_DataType)
            && rule_params_n();

    }
    return false;
}
bool rule_params_n() {
    if (g_token.type == Token_ParenRight || g_token.type == Token_EOF)
        return true;
    if (g_token.type == Token_Comma) {
        get_next_token();
        return rule_params_n();
    }
    return false;
}
bool rule_returnExpr() {
    switch (g_token.type) {
        case Token_Whitespace:
            return g_token.attribute.has_eol;
        case Token_EOF:
        case Token_BracketRight:
            return true;
        default:
            return expr_parser_begin(g_token);
    }
}
bool rule_ifCondition() {
    if (g_token.type == Token_Keyword && g_token.attribute.keyword == Keyword_Let) {
        get_next_token();
    }
    return expr_parser_begin(g_token);
}
bool rule_else() {
    if (g_token.type == Token_Keyword && g_token.attribute.keyword == Keyword_Else) {
        get_next_token();
        return check_token(Token_BracketLeft)
            && rule_statementList()
            && check_token(Token_BracketRight)
            && rule_statementList();
    }
    if (g_token.type == Token_EOF)
        return true;
    if (g_token.type == Token_Whitespace)
        return g_token.attribute.has_eol;
    return false;
}
bool rule_assignType() {
    switch (g_token.type) {
        case Token_Whitespace:
            return g_token.attribute.has_eol;
        case Token_EOF:
        case Token_Equal:
            return true;
        case Token_DoubleColon:
            get_next_token();
            return check_token(Token_DataType);
        default:
            return false;
    }
}
bool rule_assignExpr() {
    switch (g_token.type) {
        case Token_Whitespace:
            return g_token.attribute.has_eol;
        case Token_EOF:
            return true;
        case Token_Equal:
            get_next_token();
            return expr_parser_begin(g_token);
        default:
            return false;
    }
}

