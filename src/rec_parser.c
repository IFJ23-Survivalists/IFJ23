/**
 * @file rec_parser.c
 * @brief Recursive parser logic.
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#include "rec_parser.h"
#include "expr_parser.h"
#include "error.h"
#include "scanner.h"
#include "symtable.h"
#include "to_string.h"

// Shorthand for checking if there was newline before the current token.
#define HAS_EOL (g_parser.token_ws.type == Token_Whitespace && g_parser.token_ws.attribute.has_eol)
#define TOK_ID_STR g_parser.token.attribute.data.value.string.data
#define IS_NIL(data) ((data).type == DataType_Undefined && (data).is_nil)

/* Forward declarations for rule functions, so that they can call each other without issues */
bool rule_statementList();
bool rule_statementSeparator();
bool rule_statement();
bool rule_ifStatement();
bool rule_returnExpr();
bool rule_ifCondition(bool is_let);
bool rule_else();
bool rule_assignType(DataType* attr_dt);
bool rule_assignExpr(VariableSymbol* var, const char* id_name);
bool rule_elseIf();

// Stores, which function we are currently processing, so that we can do semantic checks,
// like:
//     - Checking for function declaration inside declarations
//     - Finding the function to use for return statement checks.
char* g_current_func;

// Entry point to recursive parsing.
bool rec_parser_begin() {
    g_current_func = NULL;
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

bool is_maybe_datatype(DataType dt) { return dt >= DataType_MaybeInt && dt <= DataType_MaybeBool; }
DataType maybe_to_normal(DataType maybe_dt) {
    MASSERT(is_maybe_datatype(maybe_dt), "");
    return maybe_dt - DataType_MaybeInt;
}

bool assign_types_compatible(DataType left, const Data* right) {
    if (IS_NIL(*right))
        return is_maybe_datatype(left);
    if (left == right->type)
        return true;
    if (is_maybe_datatype(left)) {
        // "Maybe types" and their counterparts are compatible.
        return maybe_to_normal(left) == right->type;
    }

    return false;
}

/**
 * @brief Handle any "= <expr>" assignments.
 * @param var Variable to assign to.
 * @param id_name Name of the variable to assign to. We need this during code generation.
 * @note We expect g_parser.token to contain the token AFTER the '='.
 * @note We expect the var->allow_modification to be TRUE to be able to assign. Otherwise error is raised.
 */
bool assign_expr(VariableSymbol* var, const char* id_name) {
    if (var->allow_modification == false) {
        undef_fun_err("Cannot assign to variable `" COL_Y("%s") "` defined using the `" COL_C("let") "` keyword.", id_name);
        return false;
    }

    // Call <expr> evaluation.
    Data expr_data;
    CALL_RULEp(expr_parser_begin, &expr_data);
    MASSERT(expr_data.type != DataType_Undefined || expr_data.is_nil, "When expr result is of type Undefined and isn't `nil`, there should be error 8 in expr parsing.");

    // When the Datatype is undefined, we assign the expr's datatype to it.
    if (var->type == DataType_Undefined) {
        // Check for when the <expr> is nil and var type is to be deduced. --> Error 8
        if (IS_NIL(expr_data)) {
            unknown_type_err("Could not deduce type of variable `" COL_Y("%s") "` from `" COL_C("nil") "`.", id_name);
            return false;
        }
        var->type = expr_data.type;
    }
    // Otherwise check the datatype compatibility.
    else if (!assign_types_compatible(var->type, &expr_data)) {
        expr_type_err("Type mismatch. Cannot assign value of type `" COL_Y("%s") "` to variable `" BOLD("%s") "` of type `" COL_Y("%s") "`.",
                     IS_NIL(expr_data) ? "nil" : datatype_to_string(expr_data.type), id_name, datatype_to_string(var->type));
        return false;
    }

    // Mark value as initialized. Assign call always initialized the variable if successful.
    var->is_initialized = true;

    // TODO: Code generation for Maybe and non-Maybe types.

    return true;
}

// Check if the variable already is in the top symbol table
bool define_variable_check(const char* name) {
    // Check if there is already a function with the same name.
    if (symtable_get_function(symstack_bottom(), name) != NULL) {
        undef_fun_err("Cannot define variable `" COL_Y("%s") "`. There is already a function with the same name.", name);
        return false;
    }

    NodeType* symtype = symtable_get_symbol_type(symstack_top(), name);
    if (symtype) {      // Symbol found
        undef_fun_err("Cannot define `" COL_Y("%s") "`. There is already a %s with the same name.",
                name, *symtype == NodeType_Variable ? "variable" : "function");
        return false;
    }
    return true;
}

bool handle_let_statement() {
    const char* id_name = TOK_ID_STR;
    CHECK_TOKEN(Token_Identifier, "Unexpected token `%s` after the `let` keyword. Expected identifier.", TOK_STR);

    if (!define_variable_check(id_name))
        return false;
    VariableSymbol var;
    variable_symbol_init(&var);

    CALL_RULEp(rule_assignType, &var.type);
    CHECK_TOKEN(Token_Equal, "Unexpected token `%s` after `let` assign statement. Expected `=`.", TOK_STR);

    // Assign value to this variable.
    var.allow_modification = true;
    if (!assign_expr(&var, id_name))
        return false;
    var.allow_modification = false;

    // Create variable in the top-most symbol table.
    if (!symtable_insert_variable(symstack_top(), id_name, var)) {
        SET_INT_ERROR(IntError_Runtime, "Could not insert variable into symbol table.");
        return false;
    }

    return true;
}

bool handle_var_statement() {
    const char* id_name = TOK_ID_STR;
    CHECK_TOKEN(Token_Identifier, "Unexpected token `%s` after the `var` keyword. Expected identifier.", TOK_STR);

    if (!define_variable_check(id_name))
        return false;
    VariableSymbol var;
    var.allow_modification = true;
    var.is_initialized = false;

    CALL_RULEp(rule_assignType, &var.type);
    CALL_RULEp(rule_assignExpr, &var, id_name);

    // Insert variable into the symboltable.
    if (!symtable_insert_variable(symstack_top(), id_name, var)) {
        SET_INT_ERROR(IntError_Runtime, "Could not insert variable into symbol table.");
        return false;
    }

    return true;
}

bool handle_func_statement() {
    // Skip until bracket left.
    char* func_id = TOK_ID_STR;
    while (g_parser.token.type != Token_BracketLeft)
        parser_next_token();
    parser_next_token();

    // Push symtable for this function.
    symstack_push();

    // Push arguments to symstack
    FunctionSymbol* func = symtable_get_function(symstack_bottom(), func_id);
    MASSERT(func != NULL, "In seconds phase all function declarations should be valid.");
    for (int i = 0; i < func->param_count; i++) {
        VariableSymbol var;
        var.type = func->params[i].type;
        var.is_initialized = true;
        var.allow_modification = true;
        symtable_insert_variable(symstack_top(), func->params[i].iname.data, var);
    }


    // TODO: Code generation, we need to create a label and generate code to some buffer.

    g_current_func = func_id;
    CALL_RULE(rule_statementList);      // Process all statements inside this function

    // Check for missing return statement.
    if (g_current_func != NULL && func->return_value_type != DataType_Undefined) {
        return_err("Missing return statement in function `" COL_Y("%s") "`.", func_id);
        g_current_func = NULL;      // Maybe for future error recovery.
        return false;
    }

    symstack_pop();

    CHECK_TOKEN(Token_BracketRight, "Unexpected token `%s` at the end of function definition. Expected `}`.", TOK_STR);
    CALL_RULE(rule_statementList);
    return true;
}

bool handle_while_statement() {
    CHECK_TOKEN(Token_ParenLeft, "Unexpected token `%s` after the `While` keyword. Expected `(`.", TOK_STR);

    Data expr_data;
    CALL_RULEp(expr_parser_begin, &expr_data);

    // Check that expr_data.type is of boolean value.
    if (expr_data.type != DataType_Bool) {
        expr_type_err("While-expression is of non-boolean type `" COL_Y("%s") "`.", datatype_to_string(expr_data.type));
        return false;
    }

    CHECK_TOKEN(Token_ParenRight, "Unexpected token `%s` at the end of While clause. Expected `)`.", TOK_STR);
    CHECK_TOKEN(Token_BracketLeft, "Unexpected token `%s` after the while clause. Expected `{`.", TOK_STR);

    symstack_push();
    CALL_RULE(rule_statementList);
    symstack_pop();

    CHECK_TOKEN(Token_BracketRight, "Unexpected token `%s` at the end of while statement. Expected `}`.", TOK_STR);
    CALL_RULE(rule_statementList);
    return true;
}

// Handle the loose `ID = <expr>` or `ID()` statements.
bool handle_id_statement() {
    const char* id_name = TOK_ID_STR;
    // If ID is a function in the global symtable, then we switch to precedence analysis.
    if (symtable_get_function(symstack_bottom(), id_name)) {
        Data expr_data;
        return expr_parser_begin(&expr_data);
    }

    // When the ID isn't a function, we expect as assignment expression.
    parser_next_token();
    CHECK_TOKEN(Token_Equal, "Unexpected token `%s`. Expected function call or assign expression.", TOK_STR);

    // Search symstack for variable with this name.
    VariableSymbol* var;
    if ((var = symstack_search_variable(id_name)) == NULL) {
        undef_var_err("Symbol `" COL_Y("%s") "` is undefined.", id_name);
        return false;
    }

    // Assign to the found symbol.
    return assign_expr(var, id_name);
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
        case Token_Identifier:
            return handle_id_statement();
        default:
            syntax_err("Unexpected token `%s` at the start of statement list.", TOK_STR);
            return false;
    }
}

bool rule_returnExpr() {
    // Check, if we are inside a function.
    if (g_current_func == NULL) {
        return_err("Invalid " COL_Y("return") " statement outside function definition.");
        return false;
    }
    FunctionSymbol* func = symtable_get_function(symstack_bottom(), g_current_func);
    MASSERT(func != NULL, "In second phase, g_current_func must always be in symtable.");

    switch (g_parser.token.type) {
        case Token_EOF:
        case Token_BracketRight: {
            // Return has no <expr>, so check if function returns nothing.
            if (func->return_value_type != DataType_Undefined) {
                return_err("Missing value of type `" COL_C("%s") "` after the `" COL_Y("return") "` keyword.",
                           datatype_to_string(func->return_value_type));
                return false;
            }
            g_current_func = NULL;
            return true;
        }
        default: {
            if (func->return_value_type == DataType_Undefined) {
                // FIXME: We COULD check if there is token of type that is not part of an expression.
                return_err("Invalid expression after the `" COL_Y("return") "` statement. Function `" BOLD("%s") "` doesn't return anything. Expected `}`.",
                            g_current_func);
                return false;
            }
            Data expr_data;
            CALL_RULEp(expr_parser_begin, &expr_data);
            MASSERT(expr_data.type != DataType_Undefined || expr_data.is_nil, "We don't support expr result with DataType_Undefined and is_nil == false.");

            // Check if expr's datatype matched the funcion's datatype.
            if (!assign_types_compatible(func->return_value_type, &expr_data)) {
                fun_type_err("Cannot return value of type `" COL_Y("%s") "` from function `" BOLD("%s") "() -> " COL_Y("%s") "`.",
                        IS_NIL(expr_data) ? "nil" : datatype_to_string(expr_data.type), g_current_func, datatype_to_string(func->return_value_type));
                return false;
            }

            // TODO: Code generation.

            // Set current func to NULL, so that we can check in `handle_func_statement()`, if the function has a return statement.
            g_current_func = NULL;
            return true;
        }
    }
}

bool rule_ifStatement() {
    // Push new symtable here, because the potential if-let statement
    // will need to create a new variable {new because we access only a
    // variable defined using `let` statement, so we don't need reference}.
    symstack_push();
    switch (g_parser.token.type) {
        case Token_ParenLeft:
            parser_next_token();
            CALL_RULEp(rule_ifCondition, false);
            break;
        case Token_Let:
            parser_next_token();
            CALL_RULEp(rule_ifCondition, true);
            break;
        default:
            syntax_err("Unexpected token `%s` after the `if` keyword. Expected `(` or `let`.", TOK_STR);
            return false;
    }
    CHECK_TOKEN(Token_BracketLeft, "Unexpected token `%s`. Expected `{`.", TOK_STR);
    CALL_RULE(rule_statementList);
    CHECK_TOKEN(Token_BracketRight, "Unexpected token `%s` after statement list at the end of `if` statement. Expected `}`.", TOK_STR);
    symstack_pop();
    CALL_RULE(rule_else);
    return true;
}

bool rule_ifCondition(bool is_let) {
    if (is_let) {
        const char* id_name = TOK_ID_STR;
        CHECK_TOKEN(Token_Identifier, "Unexpected token `" COL_Y("%s") "` in if-let statement. Expected indentifier.");

        // Check if identifier is non-modifiable variable --> otherwise error 7 (expr_type_err)
        VariableSymbol* var = symstack_search_variable(id_name);
        if (!var || !var->is_initialized) {
            undef_var_err("Variable `" COL_Y("%s") "` is not %s.", id_name, var ? "initialized" : "defined");
            return false;
        } else if (var->allow_modification) {
            expr_type_err("Cannot use non-constant variable `" COL_Y("%s") "` in a if-let statement.", id_name);
            return false;
        }

        // TODO: Retype the variable as non-maybe type in the new frame.
        // TODO: Add code for checking the `nil` value of var. If it is `nil`,
        //       then we need to jump after this if statement.
        // Either way, we need to add non-maybe type to symtable, because we need to reference
        // the correct variables in the if statement.
        if (is_maybe_datatype(var->type)) {
            VariableSymbol new_var = *var;
            new_var.type = maybe_to_normal(var->type);
            symtable_insert_variable(symstack_top(), id_name, new_var);
        } // NOTE: If variable is already of normal type, then we don't need to duplicate it.
    } else {
        Data expr_data;
        CALL_RULEp(expr_parser_begin, &expr_data);

        if (expr_data.type != DataType_Bool) {
            expr_type_err("If-expression is of non-boolean type " COL_Y("%s") ".", datatype_to_string(expr_data.type));
            return false;
        }

        // TODO: Add code for checking the expression result and jumping if needed.

        CHECK_TOKEN(Token_ParenRight, "Unexpected token `" COL_Y("%s") "` at the end of if statement. Expected `" COL_C(")") "`.", TOK_STR);
    }
    return true;
}

bool rule_else() {
    switch (g_parser.token.type) {
        case Token_EOF:  case Token_BracketRight:
        case Token_If:   case Token_Let:
        case Token_Var:  case Token_While:
        case Token_Func: case Token_Return:
            return rule_statementList();        // For the statemens right after '}' on the same line.
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

            // Create local frame for else statement.
            symstack_push();
            CALL_RULE(rule_statementList);
            symstack_pop();

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

// assignType -> e | : DataType
bool rule_assignType(DataType* attr_dt) {
    switch (g_parser.token.type) {
        case Token_EOF:
        case Token_BracketRight:
        case Token_Equal:
            *attr_dt = DataType_Undefined;
            return true;
        case Token_DoubleColon:
            parser_next_token();
            *attr_dt = g_parser.token.attribute.data_type;
            CHECK_TOKEN(Token_DataType, "Unexpected token `%s` in type specification. Expected `DataType`.", TOK_STR);
            MASSERT(*attr_dt != DataType_Undefined, "Huh? We should never get DataType_Undefined from the lexer!");
            return true;
        default:
            if (HAS_EOL) {
                *attr_dt = DataType_Undefined;
                return true;
            }
            syntax_err("Unexpected token `%s`. Expected one of `EOF`, `EOL`, `}`, `:`, `=`.", TOK_STR);
            return false;
    }
}

bool rule_assignExpr(VariableSymbol* var, const char* id_name) {
    switch (g_parser.token.type) {
        case Token_EOF:
        case Token_BracketRight:    // When there is no assign in the `var` statement.
            var->is_initialized = false;
            return true;
        case Token_Equal:
            parser_next_token();
            return assign_expr(var, id_name);
        default:
            if (HAS_EOL) {
                var->is_initialized = false;
                return true;
            }
            syntax_err("Unexpected token `%s`. Expected one of `EOL`, `EOF`, `}`, `=`.", TOK_STR);
            return false;
    }
}

