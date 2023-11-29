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
#include "to_string.h"

// Check if g_parser.token.type is `tok` and if not, set syntax_err with given msg.
#define CHECK_TOKEN(tok, errmsg) \
    if (g_parser.token.type != tok) { \
        syntax_err(errmsg); \
        return false; \
    } \
    parser_next_token()

// Check if g_parser.token.type is `tok` and if not, set syntax_err with given msg format.
#define CHECK_TOKENf(tok, errmsgfmt, ...) \
    if (g_parser.token.type != tok) { \
        syntax_errf(errmsgfmt, __VA_ARGS__); \
        return false; \
    } \
    parser_next_token()

// Call given rule function and return false if the rule fails.
#define CALL_RULE(rule) if (!rule()) return false
// Call given rule function with parameters and return false if the rule fails.
#define CALL_RULEp(rule, ...) if (!rule(__VA_ARGS__)) return false
// Shorthand for getting the string representation of token.
#define TOK_STR token_to_string(&g_parser.token)
// Shorthand for checking if there was newline before the current token.
#define HAS_EOL (g_parser.token_ws.type == Token_Whitespace && g_parser.token_ws.attribute.has_eol)

/* Forward declarations for rule functions, so that they can call each other without issues */
bool rule_statementList();
bool rule_statementSeparator();
bool rule_statement();
bool rule_funcReturnType();
bool rule_ifStatement();
bool rule_params();
bool rule_params_n();
bool rule_returnExpr();
bool rule_ifCondition();
bool rule_else();
bool rule_assignType();
bool rule_assignExpr();
bool rule_elseIf();

// Entry point to recursive parsing.
bool rec_parser_begin(ParserMode mode) {
    (void)mode;
    parser_next_token();
    return rule_statementList();
}

bool rule_statementList() {
    switch (g_parser.token.type) {
        case Token_If:
        case Token_Let:
        case Token_Var:
        case Token_While:
        case Token_Func:
        case Token_Return:
        case Token_Identifier:
            CALL_RULE(rule_statement);
            CALL_RULE(rule_statementSeparator);
            CALL_RULE(rule_statementList);
            return true;
        case Token_EOF:
        case Token_BracketRight:
            return true;
        default:
            break;
    }
    syntax_errf("Unexpected token `%s` in statement list.", TOK_STR);
    return false;
}

bool rule_statementSeparator() {
    if (HAS_EOL || g_parser.token.type == Token_BracketRight || g_parser.token.type == Token_EOF) {
        // NOTE: We don't want to consume the '}' here, so that it can be processed
        //       by the end of `while` and such.
        return true;
    }
    syntax_errf("Unexpected token `%s` between statements. Expected `EOL`, `}` or `EOF`.", TOK_STR);
    return false;
}

bool rule_ifStatement() {
    switch (g_parser.token.type) {
        case Token_ParenLeft:
        case Token_Let:
            CALL_RULE(rule_ifCondition);
            CHECK_TOKENf(Token_BracketLeft, "Unexpected token `%s`. Expected `{`.", TOK_STR);
            CALL_RULE(rule_statementList);
            CHECK_TOKENf(Token_BracketRight, "Unexpected token `%s` after statement list at the end of `if` statement. Expected `}`.", TOK_STR);
            CALL_RULE(rule_else);
            return true;
        default:
            syntax_errf("Unexpected token `%s` after the `if` keyword. Expected `(` or `let`.", TOK_STR);
            return false;
    }
}

bool handle_let_statement() {
    CHECK_TOKENf(Token_Identifier, "Unexpected token `%s` after the `let` keyword. Expected identifier.", TOK_STR);
    CALL_RULE(rule_assignType);
    CHECK_TOKENf(Token_Equal, "Unexpected token `%s` in assign statement. Expected `=`.", TOK_STR);
    Data expr_data;
    return expr_parser_begin(&expr_data);
}

bool handle_var_statement() {
    CHECK_TOKENf(Token_Identifier, "Unexpected token `%s` after the `var` keyword. Expected identifier.", TOK_STR);
    CALL_RULE(rule_assignType);
    CALL_RULE(rule_assignExpr);
    return true;
}

bool handle_func_statement() {
    CHECK_TOKENf(Token_Identifier, "Unexpected token `%s` after the `func` keyword. Expected name of the function.", TOK_STR);
    CHECK_TOKENf(Token_ParenLeft, "Unexpected token `%s` after the function name. Expected `(`.", TOK_STR);
    CALL_RULE(rule_params);
    CHECK_TOKENf(Token_ParenRight, "Unexpected token `%s` after the function parameters. Expected `)`.", TOK_STR);
    CALL_RULE(rule_funcReturnType);
    CHECK_TOKENf(Token_BracketLeft, "Unexpected token `%s` after the `DataType` token. Expected `{`.", TOK_STR);
    CALL_RULE(rule_statementList);
    CHECK_TOKENf(Token_BracketRight, "Unexpected token `%s` at the end of function definition. Expected `}`.", TOK_STR);
    CALL_RULE(rule_statementList);
    return true;
}

bool handle_while_statement() {
    CHECK_TOKENf(Token_ParenLeft, "Unexpected token `%s` after the `While` keyword. Expected `(`.", TOK_STR);
    Data expr_data;
    if (!expr_parser_begin(&expr_data)) return false;
    CHECK_TOKENf(Token_ParenRight, "Unexpected token `%s` at the end of While clause. Expected `)`.", TOK_STR);
    CHECK_TOKENf(Token_BracketLeft, "Unexpected token `%s` after the while clause. Expected `{`.", TOK_STR);
    CALL_RULE(rule_statementList);
    CHECK_TOKENf(Token_BracketRight, "Unexpected token `%s` at the end of while statement. Expected `}`.", TOK_STR);
    CALL_RULE(rule_statementList);
    return true;
}

bool rule_statement() {
    switch (g_parser.token.type) {
        case Token_If:
            parser_next_token();
            return rule_ifStatement();
        case Token_Let:
            parser_next_token();
            return handle_let_statement();
        case Token_Var:
            parser_next_token();
            return handle_var_statement();
        case Token_While:
            parser_next_token();
            return handle_while_statement();
        case Token_Func:
            parser_next_token();
            return handle_func_statement();
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
            CHECK_TOKENf(Token_Equal, "Unexpected token `%s`. Expected function call or assign expression.", TOK_STR);
            Data expr_data;
            return expr_parser_begin(&expr_data);
        }
        default:
            syntax_errf("Unexpected token `%s` at the start of statement list.", TOK_STR);
            return false;
    }
}

bool rule_funcReturnType() {
    switch (g_parser.token.type) {
        case Token_EOF:
        case Token_BracketLeft:
            return true;
        case Token_ArrowRight:
            parser_next_token();
            CHECK_TOKENf(Token_DataType, "Unexpected token `%s` after `->`. Expected `DataType`.", TOK_STR);
            return true;
        default:
            syntax_errf("Unexpected token `%s` after ')'. Expected `->` or `{`.", TOK_STR);
            return false;
    }
}

bool rule_params() {
    switch (g_parser.token.type) {
        case Token_EOF:
        case Token_ParenRight:
            return true;
        case Token_Identifier:
            parser_next_token();
            CHECK_TOKENf(Token_Identifier, "Unexpected token `%s` after the parameter name. Expected identifier.", TOK_STR);
            CHECK_TOKENf(Token_DoubleColon, "Unexpected token `%s` after inner parameter name. Expected `,` or `)`.", TOK_STR);
            CHECK_TOKENf(Token_DataType, "Unexpected token `%s`. Expected `DataType`.", TOK_STR);
            CALL_RULE(rule_params_n);
            return true;
        default:
            break;
    }
    syntax_errf("Unexpected token `%s` in function parameters.", TOK_STR);
    return false;
}

bool rule_params_n() {
    switch (g_parser.token.type) {
        case Token_EOF:
        case Token_ParenRight:
            return true;
        case Token_Comma:
            parser_next_token();
            return rule_params();
        default:
            syntax_errf("Unexpected token `%s`.", TOK_STR);
            return false;
    }
}

bool rule_returnExpr() {
    switch (g_parser.token.type) {
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
    switch (g_parser.token.type) {
        case Token_EOF:  case Token_BracketRight:
        case Token_If:   case Token_Let:
        case Token_Var:  case Token_While:
        case Token_Func: case Token_Return:
            return rule_statementList();
        case Token_Else:
            parser_next_token();
            return rule_elseIf();
        default:
            if (HAS_EOL)
                return rule_statementList();
            syntax_errf("Unexpected token `%s`. Expected `else` or end of statement.", TOK_STR);
            return false;
    }
}

bool rule_elseIf() {
    switch (g_parser.token.type) {
        case Token_BracketLeft:
            parser_next_token();
            CALL_RULE(rule_statementList);
            CHECK_TOKENf(Token_BracketRight, "Unexpected token `%s` at the end of else clause. Expected `}`.", TOK_STR);
            return rule_else();
        case Token_If:
            parser_next_token();
            return rule_ifStatement();
        default:
            syntax_errf("Unexpected token `%s` after the `else` keyword. Expected `{` or `if`.", TOK_STR);
            return false;
    }
}

bool rule_assignType() {
    switch (g_parser.token.type) {
        case Token_EOF:
        case Token_BracketRight:
        case Token_Equal:
            return true;
        case Token_DoubleColon:
            parser_next_token();
            CHECK_TOKENf(Token_DataType, "Unexpected token `%s` in type specification. Expected `DataType`.", TOK_STR);
            return true;
        default:
            if (HAS_EOL)
                return true;
            syntax_errf("Unexpected token `%s`. Expected one of `EOF`, `EOL`, `}`, `:`, `=`.", TOK_STR);
            return false;
    }
}

bool rule_assignExpr() {
    switch (g_parser.token.type) {
        case Token_EOF:
        case Token_BracketRight:
            return true;
        case Token_Equal:
            parser_next_token();
            Data expr_data;
            return expr_parser_begin(&expr_data);
        default:
            if (HAS_EOL)
                return true;
            syntax_errf("Unexpected token `%s`. Expected one of `EOL`, `EOF`, `}`, `=`.", TOK_STR);
            return false;
    }
}

