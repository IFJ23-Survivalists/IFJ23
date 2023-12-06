/**
 * @note Project: Implementace překladače imperativního jazyka IFJ23
 * @file rec_parser.c
 * @brief Recursive parser logic.
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 23/11/2023
 */
#include "rec_parser.h"
#include "error.h"
#include "expr_parser.h"
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
bool rule_returnExpr();
bool rule_ifStatement(int if_num, int after_num);
bool rule_ifCondition(bool is_let, int if_num, int after_num);
bool rule_else(int if_num, int after_num);
bool rule_elseIf(int if_num, int after_num);
bool rule_assignType(DataType* attr_dt);
bool rule_assignExpr(VariableSymbol* var, const char* id_name);

// Stores, which function we are currently processing, so that we can do semantic checks,
// like:
//     - Checking for function declaration inside declarations
//     - Finding the function to use for return statement checks.
char* g_current_func;

/// Statement counters used for uniquelly indentifying needed IFJcode23 labels.
int g_while_index;
int g_if_index;

// Entry point to recursive parsing.
bool rec_parser_begin() {
    g_current_func = NULL;
    g_while_index = g_if_index = 0;

    code_buf_set(&g_parser.var_defs_code);
    code_generation_raw(".IFJcode23");
    code_generation_raw("DEFVAR GF@ret");
    code_generation_raw("MOVE GF@ret int@0");
    code_buf_set(&g_parser.global_code);
    g_parser.currect_code = &g_parser.global_code;

    parser_next_token();
    CALL_RULEp(rule_statementList);

    code_generation_raw("LABEL exit");
    code_generation_raw("EXIT GF@ret");
    return true;
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

bool is_maybe_datatype(DataType dt) {
    return dt >= DataType_MaybeInt && dt <= DataType_MaybeBool;
}
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
 * @brief Try to implicit conversion of rhs's type to given variable's.
 *
 * This function handles the case when we assign for example double to int. In this case
 * the types aren't compatible by default, however we support implicit conversion between them.
 * If that is the case, we generete needed IFJcode23 instructions for that.
 * @param var Variable of which's type to try to match
 * @param rhs Variable to convert
 * @fixme I will proably need to add more parameters to handle the variable names and such.
 * @return 0 if no conersion can be done. 1 if Float2Int and 2 if Int2Float
 */
int assign_try_implicit_convesion(VariableSymbol* var, Data* rhs) {
    if (var->type == DataType_Int && rhs->type == DataType_Double) {
        // Generate implicit conversion of double to int.
        return 1;
    }
    if (var->type == DataType_Double && rhs->type == DataType_Int) {
        // Generate implicit conversion of int to double.
        return 2;
    }
    return 0;
}

/**
 * @brief Handle any "= <expr>" assignments.
 * @param var Variable to assign to.
 * @param id_name Name of the variable to assign to. We need this only for error messages.
 * @note We expect g_parser.token to contain the token AFTER the '='.
 * @note We expect the var->allow_modification to be TRUE to be able to assign. Otherwise error is raised.
 */
bool assign_expr(VariableSymbol* var, const char* id_name) {
    if (var->allow_modification == false) {
        undef_fun_err("Cannot assign to variable `" COL_Y("%s") "` defined using the `" COL_C("let") "` keyword.",
                      id_name);
        return false;
    }

    // Call <expr> evaluation.
    Data expr_data;
    CALL_RULEp(expr_parser_begin, &expr_data);

    if (expr_data.type == DataType_Undefined && !expr_data.is_nil) {
        unknown_type_err("Cannot assign value of type `" COL_Y("Undefined") "` to variable `" BOLD("%s") "`.", id_name);
        return false;
    }

    int impl_conv = 0;

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
        if ((impl_conv = assign_try_implicit_convesion(var, &expr_data)) == 0) {
            expr_type_err("Type mismatch. Cannot assign value of type `" COL_Y("%s") "` to variable `" BOLD(
                              "%s") "` of type `" COL_Y("%s") "`.",
                          IS_NIL(expr_data) ? "nil" : datatype_to_string(expr_data.type), id_name,
                          datatype_to_string(var->type));
            return false;
        }
    }

    // Mark value as initialized. Assign call always initialized the variable if successful.
    var->is_initialized = true;

    // TODO: MAYBE? Idk why I put this here.. Code generation for Maybe and non-Maybe types.

    // Code generation of MOVE or implicit conversion MOVE.
    Operand op1, op2;
    op1.variable.name = var->code_name.data;
    op1.variable.frame = var->code_frame;
    op2.symbol.type = SymbolType_Variable;
    op2.symbol.variable.name = "res";
    op2.symbol.variable.frame = Frame_Temporary;
    if (impl_conv != 0) {
        code_generation(impl_conv == 1 ? Instruction_Float2Int : Instruction_Int2Float, &op1, &op2, NULL);
    } else {
        code_generation(Instruction_Move, &op1, &op2, NULL);
    }

    return true;
}

// Check if the variable already is in the top symbol table
bool define_variable_check(const char* name) {
    // Check if there is already a function with the same name.
    if (symtable_get_function(symstack_bottom(), name) != NULL) {
        undef_fun_err("Cannot define variable `" COL_Y("%s") "`. There is already a function with the same name.",
                      name);
        return false;
    }

    NodeType* symtype = symtable_get_symbol_type(symstack_top(), name);
    if (symtype) {  // Symbol found
        undef_fun_err("Cannot define `" COL_Y("%s") "`. There is already a %s with the same name.", name,
                      *symtype == NodeType_Variable ? "variable" : "function");
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
    parser_variable_code_info(&var, id_name);

    FunctionSymbol* current_func = g_current_func ? symtable_get_function(symstack_bottom(), g_current_func) : NULL;
    CodeBuf* defs_buf = current_func ? &current_func->code_defs : &g_parser.var_defs_code;

    TRY_BEGIN {
        bCALL_RULEp(rule_assignType, &var.type);
        bCHECK_TOKEN(Token_Equal, "Unexpected token `%s` after `let` assign statement. Expected `=`.", TOK_STR);

        // Define the variable in IFJcode23
        Operand ifj_var;
        ifj_var.variable.name = var.code_name.data;
        ifj_var.variable.frame = var.code_frame;
        code_buf_set(defs_buf);
        code_generation(Instruction_DefVar, &ifj_var, NULL, NULL);
        code_buf_set(g_parser.currect_code);

        // Assign value to this variable.
        var.allow_modification = true;
        if (!assign_expr(&var, id_name))
            BREAK_FINAL;
        var.allow_modification = false;

        // Create variable in the top-most symbol table.
        if (!symtable_insert_variable(symstack_top(), id_name, var)) {
            SET_INT_ERROR(IntError_Runtime, "Could not insert variable into symbol table.");
            BREAK_FINAL;
        }
    }
    TRY_FINAL {
        variable_symbol_free(&var);
        return false;
    }
    TRY_END;

    return true;
}

bool handle_var_statement() {
    const char* id_name = TOK_ID_STR;
    CHECK_TOKEN(Token_Identifier, "Unexpected token `%s` after the `var` keyword. Expected identifier.", TOK_STR);

    if (!define_variable_check(id_name))
        return false;
    VariableSymbol var;
    variable_symbol_init(&var);
    var.allow_modification = true;
    var.is_initialized = false;
    parser_variable_code_info(&var, id_name);

    FunctionSymbol* current_func = g_current_func ? symtable_get_function(symstack_bottom(), g_current_func) : NULL;
    CodeBuf* defs_buf = current_func ? &current_func->code_defs : &g_parser.var_defs_code;

    TRY_BEGIN {
        // Define the variable in IFJcode23
        Operand ifj_op;
        ifj_op.variable.name = var.code_name.data;
        ifj_op.variable.frame = var.code_frame;
        code_buf_set(defs_buf);
        code_generation(Instruction_DefVar, &ifj_op, NULL, NULL);
        code_buf_set(g_parser.currect_code);

        bCALL_RULEp(rule_assignType, &var.type);
        bCALL_RULEp(rule_assignExpr, &var, id_name);

        // Insert variable into the symboltable.
        if (!symtable_insert_variable(symstack_top(), id_name, var)) {
            SET_INT_ERROR(IntError_Runtime, "Could not insert variable into symbol table.");
            BREAK_FINAL;
        }
    }
    TRY_FINAL {
        variable_symbol_free(&var);
        return false;
    }
    TRY_END;

    return true;
}

/* Function body generally looks like this */
// PUSHFRAME    ... Parameters into local variables
// .. Local statements ...
// DEFVAR LF@ret
// MOVE LF@ret TF@res   ... Result of return <expr> or something else
// POPFRAME     ... Local frame --> Temporary frame
// RETURN
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
        variable_symbol_init(&var);
        var.type = func->params[i].type;
        var.is_initialized = true;
        var.allow_modification = true;
        string_concat_c_str(&var.code_name, func->params[i].code_name.data);
        var.code_frame = Frame_Local;
        symtable_insert_variable(symstack_top(), func->params[i].iname.data, var);
    }
    parser_scope_function(func);

    code_buf_set(&func->code_defs);
    // Generate label indentifying this function.
    Operand op = {.label = func->code_name.data};
    code_generation(Instruction_Label, &op, NULL, NULL);
    // We get parameters on the temporary frame, so we need to convert it to local frame,
    // to get them as local variables.
    code_generation(Instruction_PushFrame, NULL, NULL, NULL);
    code_buf_set(&func->code);

    g_current_func = func_id;
    CALL_RULE(rule_statementList);  // Process all statements inside this function

    // Check for missing return statement.
    if (g_current_func != NULL && func->return_value_type != DataType_Undefined) {
        return_err("Missing return statement in function `" COL_Y("%s") "`.", func_id);
        g_current_func = NULL;  // Maybe for future error recovery.
        return false;
    }

    code_generation(Instruction_PopFrame, NULL, NULL, NULL);
    code_generation(Instruction_Return, NULL, NULL, NULL);
    parser_scope_global();
    symstack_pop();

    CHECK_TOKEN(Token_BracketRight, "Unexpected token `%s` at the end of function definition. Expected `}`.", TOK_STR);
    CALL_RULE(rule_statementList);
    return true;
}

bool handle_while_statement() {
    g_while_index++;  // Increment while counter to get unique while id.
    code_generation_raw("LABEL while%i_begin", g_while_index);

    Data expr_data;
    CALL_RULEp(expr_parser_begin, &expr_data);

    // Check that expr_data.type is of boolean value.
    if (expr_data.type != DataType_Bool) {
        expr_type_err("While-expression is of non-boolean type `" COL_Y("%s") "`.", datatype_to_string(expr_data.type));
        return false;
    }

    // Generate code for checking the while result and jumping if necessary.
    code_generation_raw("JUMPIFNEQ while%i_end TF@res bool@true", g_while_index);

    CHECK_TOKEN(Token_BracketLeft, "Unexpected token `%s` after the while clause. Expected `{`.", TOK_STR);

    symstack_push();
    CALL_RULE(rule_statementList);
    symstack_pop();

    // Jump to beginning of the while to check the condition.
    code_generation_raw("JUMP while%i_begin", g_while_index);
    // Label for code after this while statement.
    code_generation_raw("LABEL while%i_end", g_while_index);

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
            return rule_ifStatement(-1, 0);
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
            break;
        }
        default: {
            // Return has `<expr>`, so check if function return something.
            if (func->return_value_type == DataType_Undefined) {
                // FIXME: We COULD check if there is token of type that is not part of an expression. But those are dead
                // code.
                return_err("Invalid expression after the `" COL_Y("return") "` statement. Function `" BOLD(
                               "%s") "` doesn't return anything. Expected `}`.",
                           g_current_func);
                return false;
            }

            // Evaluate the `<expr>` statement.
            Data expr_data;
            CALL_RULEp(expr_parser_begin, &expr_data);
            MASSERT(expr_data.type != DataType_Undefined || expr_data.is_nil,
                    "We don't support expr result with DataType_Undefined and is_nil == false.");

            // Check if expr's datatype matched the funcion's datatype.
            if (!assign_types_compatible(func->return_value_type, &expr_data)) {
                fun_type_err("Cannot return value of type `" COL_Y("%s") "` from function `" BOLD("%s") "() -> " COL_Y(
                                 "%s") "`.",
                             IS_NIL(expr_data) ? "nil" : datatype_to_string(expr_data.type), g_current_func,
                             datatype_to_string(func->return_value_type));
                return false;
            }

            // Create a return value variable and move `<expr>` result into it.
            code_generation_raw("DEFVAR LF@ret");
            code_generation_raw("MOVE LF@ret TF@res");

            // Set current func to NULL, so that we can check in `handle_func_statement()`, if the function has a return
            // statement.
            g_current_func = NULL;
            break;
        }
    }
    return true;
}

bool rule_ifStatement(int if_num, int after_num) {
    if (if_num == -1) {
        if_num = ++g_if_index;
        after_num = 0;
    }

    // Push new symtable here, because the potential if-let statement
    // will need to create a new variable {new because we access only a
    // variable defined using `let` statement, so we don't need reference}.
    symstack_push();
    switch (g_parser.token.type) {
        case Token_Let:
            parser_next_token();
            CALL_RULEp(rule_ifCondition, true, if_num, after_num);
            break;
        default:
            CALL_RULEp(rule_ifCondition, false, if_num, after_num);
            break;
    }
    CHECK_TOKEN(Token_BracketLeft, "Unexpected token `%s`. Expected `{`.", TOK_STR);
    CALL_RULE(rule_statementList);
    CHECK_TOKEN(Token_BracketRight,
                "Unexpected token `%s` after statement list at the end of `if` statement. Expected `}`.", TOK_STR);
    code_generation_raw("JUMP if%i_end", if_num);
    symstack_pop();
    CALL_RULEp(rule_else, if_num, after_num);
    return true;
}

bool rule_ifCondition(bool is_let, int if_num, int after_num) {
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

        // Checking the `nil` value of var. If it is `nil`,
        // then we need to jump after this if statement.
        code_generation_raw("JUMPIFEQ if%i_after%i %s@%s nil@nil", if_num, after_num, frame_to_string(var->code_frame),
                            var->code_name.data);

        // Either way, we need to add non-maybe type to symtable, because we need to reference
        // the correct variables in the if statement.
        if (is_maybe_datatype(var->type)) {
            VariableSymbol new_var;
            variable_symbol_init(&new_var);
            new_var.is_initialized = var->is_initialized;
            new_var.allow_modification = var->allow_modification;
            new_var.type = maybe_to_normal(var->type);
            // We just reference the original variable, but this time we semantically treat it as without Maybe type.
            string_concat_c_str(&new_var.code_name, var->code_name.data);
            new_var.code_frame = var->code_frame;
            symtable_insert_variable(symstack_top(), id_name, new_var);
        }  // NOTE: If variable is already of normal type, then we don't need to duplicate it.
    } else {
        Data expr_data;
        CALL_RULEp(expr_parser_begin, &expr_data);

        if (expr_data.type != DataType_Bool) {
            expr_type_err("If-expression is of non-boolean type " COL_Y("%s") ".", datatype_to_string(expr_data.type));
            return false;
        }

        // Add code for checking the expression result and jumping if needed.
        code_generation_raw("JUMPIFNEQ if%i_after%i TF@res bool@true", if_num, after_num);
    }
    return true;
}

bool rule_else(int if_num, int after_num) {
    code_generation_raw("LABEL if%i_after%i", if_num, after_num++);
    switch (g_parser.token.type) {
        case Token_EOF:
        case Token_BracketRight:
        case Token_If:
        case Token_Let:
        case Token_Var:
        case Token_While:
        case Token_Func:
        case Token_Return:
        case Token_Identifier:
            code_generation_raw("LABEL if%i_end", if_num);
            return rule_statementList();  // For the statemens right after '}' on the same line.
        case Token_Else:
            parser_next_token();
            return rule_elseIf(if_num, after_num);
        default:
            if (HAS_EOL) {
                code_generation_raw("LABEL if%i_end", if_num);
                return rule_statementList();
            }
            syntax_err("Unexpected token `%s`. Expected `else` or end of statement.", TOK_STR);
            return false;
    }
}

bool rule_elseIf(int if_num, int after_num) {
    switch (g_parser.token.type) {
        case Token_BracketLeft:
            parser_next_token();

            // Create local frame for else statement.
            symstack_push();
            CALL_RULE(rule_statementList);
            symstack_pop();

            CHECK_TOKEN(Token_BracketRight, "Unexpected token `%s` at the end of else clause. Expected `}`.", TOK_STR);
            code_generation_raw("LABEL if%i_end", if_num);
            return rule_statementList();
        case Token_If:
            parser_next_token();
            return rule_ifStatement(if_num, after_num);
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
        case Token_BracketRight:  // When there is no assign in the `var` statement.
            break;
        case Token_Equal:
            parser_next_token();
            return assign_expr(var, id_name);
        default:
            if (HAS_EOL)
                break;
            syntax_err("Unexpected token `%s`. Expected one of `EOL`, `EOF`, `}`, `=`.", TOK_STR);
            return false;
    }

    // When we are there, it means that the variable doesn't have ` = <expr>` statement.
    // In this case we initialize this variable to `nil` if it is of `maybe type`. Otherwise
    // it is uninitialized.
    if ((var->is_initialized = is_maybe_datatype(var->type))) {
        // Initialize the variable to `nil`.
        Operand op1, op2;
        op1.variable.name = var->code_name.data;
        op1.variable.frame = var->code_frame;
        op2.symbol.type = SymbolType_Constant;
        op2.symbol.constant.type = DataType_Undefined;
        op2.symbol.constant.is_nil = true;
        code_generation(Instruction_Move, &op1, &op2, NULL);
    }

    return true;
}
