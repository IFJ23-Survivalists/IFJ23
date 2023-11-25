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

// Current non-whitespace token.
Token g_token;
// Whitespace token if the `g_token` has a whitespace before it.
// Otherwise it is equal to g_token.
Token g_token_ws;

inline void get_next_token() {
    g_token_ws = scanner_advance();
    g_token = g_token_ws.type == Token_Whitespace
            ? scanner_advance()
            : g_token_ws;
}

bool rule_statementList();
bool rule_statementSeparator();
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
        case Token_If:
        case Token_Let:
        case Token_Var:
        case Token_While:
        case Token_Func:
        case Token_Return: {
            bool stmt_res = rule_statement();
            bool sep_res = rule_statementSeparator();
            get_next_token();
            return stmt_res && sep_res && rule_statementList();
        } break;
        case Token_Identifier: {
            bool stmt_res = rule_statement();
            get_next_token();
            bool has_eol = true;
            get_next_token();
            return stmt_res && has_eol && rule_statementList();
        } break;
        case Token_Whitespace:
            if (g_token_ws.attribute.has_eol)
                return true;
            break;
        case Token_EOF:
        case Token_BracketRight:
            return true;
        default:
            break;
    }

    set_error(Error_Syntax);
    return false;
}

bool rule_statementSeparator() {
    if (g_token_ws.type == Token_Whitespace) {
        if (g_token.attribute.has_eol)
            return true;
    }
    if (g_token.type == Token_BracketRight) {
        // NOTE: We don't want to consume the '}' here, so that it can be processed
        //       by the end of `while` and such.
        return true;
    }
    set_error(Error_Syntax);
    return false;
}

bool rule_statement() {
    switch (g_token.type) {
        case Token_If:
            get_next_token();
            return rule_ifCondition()
                && check_token(Token_BracketLeft)
                && rule_statementList()
                && check_token(Token_BracketRight)
                && rule_statementList();
        case Token_Let:
            get_next_token();
            return check_token(Token_Identifier)
                && rule_assignType()
                && check_token(Token_Equal)
                && expr_parser_begin(g_token);
        case Token_Var:
            get_next_token();
            return check_token(Token_Identifier)
                && rule_assignType()
                && rule_assignExpr();
        case Token_While:
            get_next_token();
            return check_token(Token_ParenLeft)
                && expr_parser_begin(g_token)
                && check_token(Token_ParenRight)
                && check_token(Token_BracketLeft)
                && rule_statementList()
                && check_token(Token_BracketRight)
                && rule_statementList();
        case Token_Func:
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
        case Token_Return:
            get_next_token();
            return rule_returnExpr();
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
    set_error(Error_Syntax);
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
    set_error(Error_Syntax);
    return false;
}
bool rule_params_n() {
    if (g_token.type == Token_ParenRight || g_token.type == Token_EOF)
        return true;
    if (g_token.type == Token_Comma) {
        get_next_token();
        return rule_params_n();
    }
    set_error(Error_Syntax);
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
    if (g_token.type == Token_Let) {
        get_next_token();
    }
    return expr_parser_begin(g_token);
}
bool rule_else() {
    if (g_token.type == Token_Else) {
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
    set_error(Error_Syntax);
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
            set_error(Error_Syntax);
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
            set_error(Error_Syntax);
            return false;
    }
}

