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
#include "to_string.h"

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
bool rule_assignType(DataType* attr_dt);
bool rule_assignExpr();
bool rule_elseIf();

// Entry point to recursive parsing.
bool rec_parser_begin() {
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
    syntax_err("Unexpected token `%s` in statement list.", TOK_STR);
    return false;
}

bool rule_statementSeparator() {
    if (HAS_EOL || g_parser.token.type == Token_BracketRight || g_parser.token.type == Token_EOF) {
        // NOTE: We don't want to consume the '}' here, so that it can be processed
        //       by the end of `while` and such.
        return true;
    }
    syntax_err("Unexpected token `%s` between statements. Expected `EOL`, `}` or `EOF`.", TOK_STR);
    return false;
}

bool rule_ifStatement() {
    switch (g_parser.token.type) {
        case Token_ParenLeft:
        case Token_Let:
            CALL_RULE(rule_ifCondition);
            CHECK_TOKEN(Token_BracketLeft, "Unexpected token `%s`. Expected `{`.", TOK_STR);
            CALL_RULE(rule_statementList);
            CHECK_TOKEN(Token_BracketRight, "Unexpected token `%s` after statement list at the end of `if` statement. Expected `}`.", TOK_STR);
            CALL_RULE(rule_else);
            return true;
        default:
            syntax_err("Unexpected token `%s` after the `if` keyword. Expected `(` or `let`.", TOK_STR);
            return false;
    }
}

bool assign_types_compatible(DataType left, DataType right) {
    if (left == right)
        return true;
    if (left >= DataType_MaybeInt && left <= DataType_MaybeBool) {
        // "Maybe types" and their counterparts are compatible.
        return (left - DataType_MaybeInt) == right || right == DataType_Nil;
    }
    return false;
}

bool handle_let_statement() {
    TokenAttribute attr = g_parser.token.attribute;
    CHECK_TOKEN(Token_Identifier, "Unexpected token `%s` after the `let` keyword. Expected identifier.", TOK_STR);
    const char* id_name = attr.data.value.string.data;

    // Check if the variable already is in the top symbol table
    NodeType* symtype = symtable_get_symbol_type(symstack_top(), id_name);
    if (symtype) {      // Symbol found
        undef_var_err("Cannot define `" COL_Y("%s") "`. There is already a %s with the same name.", id_name);
        return false;
    }
    VariableSymbol var;
    variable_symbol_init(&var);
    var.allow_modification = false;
    var.is_defined = true;      // Let statement are always definitions. If not there is a syntax error.

    CALL_RULEp(rule_assignType, &var.type);
    CHECK_TOKEN(Token_Equal, "Unexpected token `%s` in assign statement. Expected `=`.", TOK_STR);
    Data expr_data;
    CALL_RULEp(expr_parser_begin, &expr_data);

    // Deduce the type if needed
    if (var.type == DataType_Nil) {
        var.type = expr_data.type;
    }

    // Check if both left and right assign operators have the same type.
    if (!assign_types_compatible(var.type, expr_data.type)) {
        semantic_err("Type mismatch. Cannot assign variable of type " COL_Y("%s") " to type "
                    COL_Y("%s") ".", datatype_to_string(expr_data.type), datatype_to_string(var.type));
        return false;
    }
    // TODO: Assign of `nil` to `Maybe` types.

    // Create variable in the top-most symbol table.
    if (!symtable_insert_variable(symstack_top(), id_name, var)) {
        SET_INT_ERROR(IntError_Runtime, "Could not insert variable into symbol table.");
        return false;
    }

    return true;
}

bool handle_var_statement() {
    CHECK_TOKEN(Token_Identifier, "Unexpected token `%s` after the `var` keyword. Expected identifier.", TOK_STR);
    DataType dt;
    CALL_RULEp(rule_assignType, &dt);
    CALL_RULE(rule_assignExpr);
    return true;
}

bool handle_func_statement() {
    // NOTE: Handled by collect phase.
    CHECK_TOKEN(Token_Identifier, "Unexpected token `%s` after the `func` keyword. Expected name of the function.", TOK_STR);
    CHECK_TOKEN(Token_ParenLeft, "Unexpected token `%s` after the function name. Expected `(`.", TOK_STR);
    CALL_RULE(rule_params);
    CHECK_TOKEN(Token_ParenRight, "Unexpected token `%s` after the function parameters. Expected `)`.", TOK_STR);
    CALL_RULE(rule_funcReturnType);

    // TODO
    CHECK_TOKEN(Token_BracketLeft, "Unexpected token `%s` after the `DataType` token. Expected `{`.", TOK_STR);
    CALL_RULE(rule_statementList);
    CHECK_TOKEN(Token_BracketRight, "Unexpected token `%s` at the end of function definition. Expected `}`.", TOK_STR);
    CALL_RULE(rule_statementList);
    return true;
}

bool handle_while_statement() {
    CHECK_TOKEN(Token_ParenLeft, "Unexpected token `%s` after the `While` keyword. Expected `(`.", TOK_STR);
    Data expr_data;
    CALL_RULEp(expr_parser_begin, &expr_data);
    CHECK_TOKEN(Token_ParenRight, "Unexpected token `%s` at the end of While clause. Expected `)`.", TOK_STR);
    CHECK_TOKEN(Token_BracketLeft, "Unexpected token `%s` after the while clause. Expected `{`.", TOK_STR);
    CALL_RULE(rule_statementList);
    CHECK_TOKEN(Token_BracketRight, "Unexpected token `%s` at the end of while statement. Expected `}`.", TOK_STR);
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
            NodeType* id_type = symtable_get_symbol_type(symstack_top(), g_parser.token.attribute.data.value.string.data);
            if (!id_type) {
                undef_var_err("Symbol `" COL_Y("%s") "` is undefined.", g_parser.token.attribute.data.value.string.data);
                return false;
            } else if (*id_type == NodeType_Function) {
                Data expr_data;
                return expr_parser_begin(&expr_data);
            }
            parser_next_token();
            CHECK_TOKEN(Token_Equal, "Unexpected token `%s`. Expected function call or assign expression.", TOK_STR);
            Data expr_data;
            return expr_parser_begin(&expr_data);
        }
        default:
            syntax_err("Unexpected token `%s` at the start of statement list.", TOK_STR);
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
            CHECK_TOKEN(Token_DataType, "Unexpected token `%s` after `->`. Expected `DataType`.", TOK_STR);
            return true;
        default:
            syntax_err("Unexpected token `%s` after ')'. Expected `->` or `{`.", TOK_STR);
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
            CHECK_TOKEN(Token_Identifier, "Unexpected token `%s` after the parameter name. Expected identifier.", TOK_STR);
            CHECK_TOKEN(Token_DoubleColon, "Unexpected token `%s` after inner parameter name. Expected `,` or `)`.", TOK_STR);
            CHECK_TOKEN(Token_DataType, "Unexpected token `%s`. Expected `DataType`.", TOK_STR);
            CALL_RULE(rule_params_n);
            return true;
        default:
            break;
    }
    syntax_err("Unexpected token `%s` in function parameters.", TOK_STR);
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
            syntax_err("Unexpected token `%s`.", TOK_STR);
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
            syntax_err("Unexpected token `%s`. Expected `else` or end of statement.", TOK_STR);
            return false;
    }
}

bool rule_elseIf() {
    switch (g_parser.token.type) {
        case Token_BracketLeft:
            parser_next_token();
            CALL_RULE(rule_statementList);
            CHECK_TOKEN(Token_BracketRight, "Unexpected token `%s` at the end of else clause. Expected `}`.", TOK_STR);
            return rule_else();
        case Token_If:
            parser_next_token();
            return rule_ifStatement();
        default:
            syntax_err("Unexpected token `%s` after the `else` keyword. Expected `{` or `if`.", TOK_STR);
            return false;
    }
}

bool rule_assignType(DataType* attr_dt) {
    switch (g_parser.token.type) {
        case Token_EOF:
        case Token_BracketRight:
        case Token_Equal:
            *attr_dt = DataType_Nil;  // nil means we have to deduce it.
            return true;
        case Token_DoubleColon:
            parser_next_token();
            *attr_dt = g_parser.token.attribute.data_type;
            CHECK_TOKEN(Token_DataType, "Unexpected token `%s` in type specification. Expected `DataType`.", TOK_STR);
            return true;
        default:
            if (HAS_EOL) {
                *attr_dt = DataType_Nil;
                return true;
            }
            syntax_err("Unexpected token `%s`. Expected one of `EOF`, `EOL`, `}`, `:`, `=`.", TOK_STR);
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
            syntax_err("Unexpected token `%s`. Expected one of `EOL`, `EOF`, `}`, `=`.", TOK_STR);
            return false;
    }
}

