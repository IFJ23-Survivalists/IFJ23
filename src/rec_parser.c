/**
 * @file rec_parser.c
 * @brief Recursive parser logic.
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#include <stdarg.h>
#include <string.h>

#include "rec_parser.h"
#include "expr_parser.h"
#include "error.h"
#include "scanner.h"
#include "parser.h"

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

bool rec_parser_begin(ParserMode mode) {
    (void)mode;
    parser_next_token();
    return rule_statementList();
}

bool check_token(TokenType tok) {
    if (g_parser.token.type == tok) {
        parser_next_token();
        return true;
    }
    set_error(Error_Syntax);
    return false;
}

bool rule_statementList() {
    switch (g_parser.token.type) {
        case Token_If:
        case Token_Let:
        case Token_Var:
        case Token_While:
        case Token_Func:
        case Token_Return: {
            bool stmt_res = rule_statement();
            bool sep_res = rule_statementSeparator();
            parser_next_token();
            return stmt_res && sep_res && rule_statementList();
        } break;
        case Token_Identifier: {
            bool stmt_res = rule_statement();
            parser_next_token();
            bool has_eol = true;
            parser_next_token();
            return stmt_res && has_eol && rule_statementList();
        } break;
        case Token_Whitespace:
            if (g_parser.token_ws.attribute.has_eol)
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
    if (g_parser.token_ws.type == Token_Whitespace) {
        if (g_parser.token.attribute.has_eol)
            return true;
    }
    if (g_parser.token.type == Token_BracketRight) {
        // NOTE: We don't want to consume the '}' here, so that it can be processed
        //       by the end of `while` and such.
        return true;
    }
    set_error(Error_Syntax);
    return false;
}

bool handle_let_statement() {
    if (g_parser.token.type != Token_Identifier) {
        set_error(Error_Syntax);
        return false;
    }
    parser_next_token();

    // Check if the ID is already in symtable.
    // if (symstack_search(&g_symstack, g_parser.token.attribute.data.value.string.data))
    Data expr_data;
    return rule_assignType()
        && check_token(Token_Equal)
        && expr_parser_begin(&expr_data);
}

bool rule_statement() {
    switch (g_parser.token.type) {
        case Token_If:
            parser_next_token();
            return rule_ifCondition()
                && check_token(Token_BracketLeft)
                && rule_statementList()
                && check_token(Token_BracketRight)
                && rule_statementList();
        case Token_Let:
            parser_next_token();
            return handle_let_statement();
        case Token_Var:
            parser_next_token();
            return check_token(Token_Identifier)
                && rule_assignType()
                && rule_assignExpr();
        case Token_While:
            parser_next_token();
            Data expr_data;
            return check_token(Token_ParenLeft)
                && expr_parser_begin(&expr_data)
                && check_token(Token_ParenRight)
                && check_token(Token_BracketLeft)
                && rule_statementList()
                && check_token(Token_BracketRight)
                && rule_statementList();
        case Token_Func:
            parser_next_token();
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
            parser_next_token();
            return rule_returnExpr();
        case Token_Identifier: {
            // Check if this ID is a function or not. Based on that select the correct rule to use.
            if (parser_tok_is_fun_id()) {
                Data expr_data;
                return expr_parser_begin(&expr_data);
            }
            parser_next_token();
            Data expr_data;
            return check_token(Token_Equal) && expr_parser_begin(&expr_data);
        }
        default:
            break;
    }
    set_error(Error_Syntax);
    return false;
}
bool rule_params() {
    if (g_parser.token.type == Token_ParenRight)
        return true;
    if (g_parser.token.type == Token_Identifier || g_parser.token.type == Token_EOF) {
        parser_next_token();
        return check_token(Token_Identifier)
            && check_token(Token_DoubleColon)
            && check_token(Token_DataType)
            && rule_params_n();

    }
    set_error(Error_Syntax);
    return false;
}

bool rule_params_n() {
    if (g_parser.token.type == Token_ParenRight || g_parser.token.type == Token_EOF)
        return true;
    if (g_parser.token.type == Token_Comma) {
        parser_next_token();
        return rule_params_n();
    }
    set_error(Error_Syntax);
    return false;
}

bool rule_returnExpr() {
    switch (g_parser.token.type) {
        case Token_Whitespace:
            return g_parser.token.attribute.has_eol;
        case Token_EOF:
        case Token_BracketRight:
            return true;
        default: {
            Data expr_data;
            return expr_parser_begin(&expr_data);
        }
    }
}

bool rule_ifCondition() {
    if (g_parser.token.type == Token_Let) {
        parser_next_token();
    }
    Data expr_data;
    return expr_parser_begin(&expr_data);
}

bool rule_else() {
    if (g_parser.token.type == Token_Else) {
        parser_next_token();
        return check_token(Token_BracketLeft)
            && rule_statementList()
            && check_token(Token_BracketRight)
            && rule_statementList();
    }
    if (g_parser.token.type == Token_EOF)
        return true;
    if (g_parser.token.type == Token_Whitespace)
        return g_parser.token.attribute.has_eol;
    set_error(Error_Syntax);
    return false;
}

bool rule_assignType() {
    switch (g_parser.token.type) {
        case Token_Whitespace:
            return g_parser.token.attribute.has_eol;
        case Token_EOF:
        case Token_Equal:
            return true;
        case Token_DoubleColon:
            parser_next_token();
            return check_token(Token_DataType);
        default:
            set_error(Error_Syntax);
            return false;
    }
}

bool rule_assignExpr() {
    switch (g_parser.token.type) {
        case Token_Whitespace:
            return g_parser.token.attribute.has_eol;
        case Token_EOF:
            return true;
        case Token_Equal:
            parser_next_token();
            Data expr_data;
            return expr_parser_begin(&expr_data);
        default:
            set_error(Error_Syntax);
            return false;
    }
}

