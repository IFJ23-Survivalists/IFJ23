/**
 * @file expr_parser.c
 * @brief Implementation for the expr_parser.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @author Matúš Moravčík, xmorav48, FIT VUT
 * @date 27/11/2023
 */
#include "expr_parser.h"
#include <string.h>
#include "function_stack.h"
#include "parser.h"
#include "pushdown.h"
#include "symtable.h"
#include "to_string.h"

#define FREE_ALL(...)                                       \
    do {                                                    \
        size_t i = 0;                                       \
        void* pta[] = {__VA_ARGS__};                        \
        for (i = 0; i < sizeof(pta) / sizeof(void*); i++) { \
            free(pta[i]);                                   \
        }                                                   \
    } while (0)

const char* RULES[] = {
    "i", "(E)", "-E", "E!", "E+E", "E*E", "E>E", "E?E", "E,E", "L,E", "i(L)", "i(E)", "i()", "i:E",
};

const Rule RULE_NAMES[] = {
    Rule_Identif,       Rule_Paren,  Rule_Prefix, Rule_Postfix,         Rule_SumSub, Rule_MulDiv,  Rule_Logic,
    Rule_NilCoalescing, Rule_ArgsEE, Rule_ArgsLE, Rule_FnArgsProcessed, Rule_FnArgs, Rule_FnEmpty, Rule_NamedArg,
};

const ComprarisonResult PRECEDENCE_TABLE[12][12] = {
    {Left, Right, Left, Left, Right, Right, Right, Left, Right, Left, Left, Left},   /* +- */
    {Left, Left, Left, Left, Right, Right, Right, Left, Right, Left, Left, Left},    /* * */
    {Right, Right, Err, Left, Right, Right, Right, Left, Right, Left, Left, Left},   /* logic ==, <, >... */
    {Right, Right, Right, Right, Right, Right, Right, Err, Right, Left, Err, Left},  /* ?? */
    {Left, Left, Left, Left, Err, Right, Right, Left, Right, Left, Left, Left},      /* pre */
    {Left, Left, Left, Left, Left, Err, Err, Left, Right, Left, Left, Left},         /* post */
    {Right, Right, Right, Err, Right, Right, Right, Equal, Right, Right, Left, Err}, /* ( */
    {Left, Left, Left, Left, Left, Left, Err, Left, Err, Left, Left, Left},          /* ) */
    {Left, Left, Left, Left, Left, Left, Equal, Left, Err, Left, Equal, Left},       /* id */
    {Right, Right, Right, Right, Right, Right, Right, Left, Right, Left, Err, Err},  /* ,*/
    {Right, Right, Right, Right, Right, Right, Right, Left, Right, Left, Err, Err},  /* : */
    {Right, Right, Right, Right, Right, Right, Right, Err, Right, Err, Err, Err}};   /* $ */

Stack g_stack;
Pushdown g_pushdown;

bool expr_parser_begin(Data* data) {
    NTerm* nterm = NULL;

    stack_init(&g_stack);
    pushdown_init(&g_pushdown);

    parse(g_parser.token, NULL);
    PushdownItem* item = pushdown_last(&g_pushdown);

    if (item != NULL)
        nterm = item->nterm;

    // check if pushdown is reduced to one nonterminal
    if (g_pushdown.first == g_pushdown.last && nterm != NULL && nterm->name == 'E') {
        data->type = nterm->type;
        data->value = nterm->value;
        stack_free(&g_stack);
        pushdown_free(&g_pushdown);
        return true;
    }

    // error occurred during parsing
    if (!got_error()) {
        syntax_err("Unexpected token: '%s'", token_to_string(&g_parser.token));
    }

    stack_free(&g_stack);
    pushdown_free(&g_pushdown);
    return false;
}

ComprarisonResult getPrecedence(PrecedenceCat pushdownItem, PrecedenceCat inputToken) {
    return PRECEDENCE_TABLE[pushdownItem][inputToken];
}

PrecedenceCat getTokenPrecedenceCategory(Token token, Token* prev_token) {
    switch (token.type) {
        case Token_Operator:
            switch (token.attribute.op) {
                case Operator_Plus:
                    return PrecendeceCat_PlusMinus;
                case Operator_Minus:
                    if (prev_token == NULL)
                        // unary
                        return PrecendeceCat_Pre;

                    switch (prev_token->type) {
                        // unary
                        case Token_ParenLeft:
                        case Token_DoubleColon:
                        case Token_Comma:
                            return PrecendeceCat_Pre;

                        // binary
                        default:
                            return PrecendeceCat_PlusMinus;
                    }

                case Operator_Multiply:
                case Operator_Divide:
                    return PrecendeceCat_MultiDiv;

                case Operator_Negation:
                    if (prev_token == NULL)
                        return PrecendeceCat_Pre;

                    switch (prev_token->type) {
                        // postfix exclamation mark
                        case Token_Identifier:
                        case Token_Data:
                        case Token_ParenRight:
                            return PrecendeceCat_Post;

                        default:
                            return PrecendeceCat_Pre;
                    }
                case Operator_DoubleQuestionMark:
                    return PrecendeceCat_NilCoalescing;

                default:
                    return PrecendeceCat_Logic;
            }

        case Token_ParenLeft:
            return PrecendeceCat_LeftPar;

        case Token_ParenRight:
            return PrecendeceCat_RightPar;

        case Token_Identifier:
        case Token_Data:
            return PrecendeceCat_Id;

        case Token_Comma:
            return PrecendeceCat_Comma;

        case Token_DoubleColon:
            return PrecendeceCat_Colon;

        default:
            return PrecendeceCat_Expr_End;
    }
    return 0;
}

char precedence_to_char(PrecedenceCat cat) {
    switch (cat) {
        case PrecendeceCat_PlusMinus:
            return '+';
        case PrecendeceCat_MultiDiv:
            return '*';
        case PrecendeceCat_Logic:
            return '>';
        case PrecendeceCat_NilCoalescing:
            return '?';
        case PrecendeceCat_Pre:
            return '-';
        case PrecendeceCat_Post:
            return '!';
        case PrecendeceCat_LeftPar:
            return '(';
        case PrecendeceCat_RightPar:
            return ')';
        case PrecendeceCat_Id:
            return 'i';
        case PrecendeceCat_Comma:
            return ',';
        case PrecendeceCat_Colon:
            return ':';
        case PrecendeceCat_Expr_End:
            return '$';
    }
    return '$';
}

PrecedenceCat char_to_precedence(char ch) {
    switch (ch) {
        case '+':
            return PrecendeceCat_PlusMinus;
        case '*':
            return PrecendeceCat_MultiDiv;
        case '>':
            return PrecendeceCat_Logic;
        case '?':
            return PrecendeceCat_NilCoalescing;
        case '-':
            return PrecendeceCat_Pre;
        case '!':
            return PrecendeceCat_Post;
        case '(':
            return PrecendeceCat_LeftPar;
        case ')':
            return PrecendeceCat_RightPar;
        case 'i':
            return PrecendeceCat_Id;
        case ',':
            return PrecendeceCat_Comma;
        case ':':
            return PrecendeceCat_Colon;
        default:
            return PrecendeceCat_Expr_End;
    }
}

void print_pushdown(Pushdown* pushdown) {
    PushdownItem* item = pushdown->first;
    printf("$");
    while (item != NULL) {
        printf("%c", item->name);
        item = pushdown_next(item);
    }
}

void parse(Token token, Token* prev_token) {
    // print_pushdown(&g_pushdown);
    PushdownItem* topmost_terminal = pushdown_search_terminal(&g_pushdown);
    PrecedenceCat topmost_terminal_prec =
        topmost_terminal ? char_to_precedence(topmost_terminal->name) : PrecendeceCat_Expr_End;
    PrecedenceCat token_prec = getTokenPrecedenceCategory(token, prev_token);
    ComprarisonResult comp_res = getPrecedence(topmost_terminal_prec, token_prec);

    // printf("\t>>In: %c", precedence_to_char(token_prec));
    // printf("\t>>TOP token: %c", precedence_to_char(topmost_terminal_prec));
    // puts("");

    switch (comp_res) {
        case Left:
            if (reduce())
                parse(token, prev_token);
            break;

        case Right: {
            PushdownItem* rule_end_marker = create_pushdown_item(NULL, NULL);
            PushdownItem* term = create_pushdown_item(&token, NULL);
            term->name = precedence_to_char(token_prec);

            pushdown_insert_after(&g_pushdown, topmost_terminal, rule_end_marker);
            pushdown_insert_last(&g_pushdown, term);

            parse(*parser_next_token(), &token);
        } break;

        case Equal: {
            PushdownItem* term = create_pushdown_item(&token, NULL);
            term->name = precedence_to_char(token_prec);

            // previous token is function identifier
            if (token.type == Token_ParenLeft) {
                if (prev_token->type == Token_Identifier) {
                    String fn_name = prev_token->attribute.data.value.string;
                    FunctionSymbol* fn = get_fn_symbol(fn_name);

                    if (fn == NULL)
                        return;  // function is undefined

                    stack_push(&g_stack, fn_name, fn);
                } else if (prev_token->type == Token_Data) {
                    syntax_err("'%s' is not callable", datatype_to_string(prev_token->attribute.data_type));
                    return;
                }
            }

            pushdown_insert_last(&g_pushdown, term);
            parse(*parser_next_token(), &token);
        } break;

        case Err:
            // reduce pushdown until it is possible
            while (reduce())
                ;
            return;
    }
}

FunctionSymbol* get_fn_symbol(String fn_name) {
    Symtable* sym = symstack_search(fn_name.data);

    if (sym == NULL) {
        undef_fun_err("Undefined function '%s'. Function must be declared or defined before use.", fn_name.data);
        return NULL;
    }

    FunctionSymbol* fs = symtable_get_function(sym, fn_name.data);
    if (fs == NULL) {
        undef_fun_err("'%s' is not a function", fn_name.data);
        return NULL;
    }
    return fs;
}

bool reduce() {
    PushdownItem* rule_operands[MAX_RULE_LENGTH];
    PushdownItem* rule_end_marker = pushdown_search_name(&g_pushdown, '|');
    int idx;

    // no rule end => no reduction possible
    if (rule_end_marker == NULL)
        return false;

    char* rule = malloc(sizeof(char) * MAX_RULE_LENGTH + 1);
    PushdownItem* item = pushdown_next(rule_end_marker);

    // get rule name and operands from top of the pushdown
    for (idx = 0; idx < MAX_RULE_LENGTH && item != NULL; idx++) {
        rule_operands[idx] = item;
        rule[idx] = item->name;
        item = pushdown_next(item);
    }
    rule[idx] = '\0';

    Rule rule_name = get_rule(rule);
    NTerm* nterm = apply_rule(rule_name, rule_operands);

    // remove the rule from pushdown
    pushdown_remove_all_from_current(&g_pushdown, rule_end_marker);

    // check if rule was applyed
    if (nterm == NULL) {
        FREE_ALL(rule);
        return false;
    }

    // push a new non-terminal to the pushdown
    PushdownItem* nterm_item = create_pushdown_item(NULL, nterm);
    nterm_item->name = nterm->name;
    pushdown_insert_last(&g_pushdown, nterm_item);

    FREE_ALL(rule);
    return true;
}

Rule get_rule(char* rule) {
    for (size_t i = 0; i < RULE_COUNT; i++) {
        if (strcmp(rule, RULES[i]) == 0)
            return RULE_NAMES[i];
    }
    return NoRule;
}

NTerm* apply_rule(Rule rule, PushdownItem** operands) {
    NTerm* nterm = malloc(sizeof(NTerm));

    // default nonterminal attributes
    nterm->name = 'E';
    nterm->is_const = false;
    nterm->is_named_arg = false;

    switch (rule) {
        case Rule_Identif:
            return reduce_identifier(operands, nterm);
        case Rule_Paren:
            return reduce_parenthesis(operands, nterm);
        case Rule_Prefix:
            return reduce_prefix(operands, nterm);
        case Rule_Postfix:
            return reduce_postfix(operands, nterm);
        case Rule_SumSub:
        case Rule_MulDiv:
            return reduce_arithmetic(operands, nterm);
        case Rule_Logic:
            return reduce_logic(operands, nterm);
        case Rule_NilCoalescing:
            return reduce_nil_coalescing(operands, nterm);
        case Rule_ArgsEE:
        case Rule_ArgsLE:
            return reduce_args(operands, nterm);
        case Rule_FnEmpty:
            return reduce_function(nterm, NULL);
        case Rule_FnArgsProcessed:;
        case Rule_FnArgs:
            return reduce_function(nterm, operands[2]->nterm);
        case Rule_NamedArg:
            return reduce_named_arg(operands, nterm);
        case NoRule:
            FREE_ALL(nterm);
            return NULL;
    }
    return nterm;
}

NTerm* reduce_identifier(PushdownItem** operands, NTerm* nterm) {
    Token* id = operands[0]->term;

    // identifier
    if (id->type == Token_Identifier) {
        char* id_name = id->attribute.data.value.string.data;
        Symtable* st = symstack_search(id_name);

        // identifier is not defined
        if (st == NULL) {
            undef_var_err("Indentifier '%s' is undefined", token_to_string(id));
            FREE_ALL(nterm);
            return NULL;
        }

        VariableSymbol* vs = symtable_get_variable(st, id_name);
        if (vs == NULL || !vs->is_initialized) {
            undef_var_err("Indentifier '%s' is undefined", token_to_string(id));
            FREE_ALL(nterm);
            return NULL;
        }

        nterm->type = vs->type;
    }
    // constant
    else {
        if (id->attribute.data.is_nil) {
            nterm->type = DataType_Undefined;

            // DataValue val;
            // val.is_nil = true;
            // nterm->value = val;
        } else
            nterm->type = id->attribute.data.type;

        nterm->is_const = true;
    }
    return nterm;
}

NTerm* reduce_parenthesis(PushdownItem** operands, NTerm* nterm) {
    NTerm* expr = operands[1]->nterm;
    nterm->type = expr->type;
    FREE_ALL(expr);
    return nterm;
}

NTerm* reduce_prefix(PushdownItem** operands, NTerm* nterm) {
    NTerm* operand = operands[1]->nterm;

    // -E
    if (operands[0]->term->attribute.op == Operator_Negation) {
        if (operand->type != DataType_Bool) {
            expr_type_err("Expected 'Bool', found '%s'.", datatype_to_string(operand->type));
            FREE_ALL(operand, nterm);
            return NULL;
        }
    }

    // !E
    else {
        if (operand->type != DataType_Int && operand->type != DataType_Double) {
            expr_type_err("Expected 'Int' or 'Double', found '%s'.", datatype_to_string(operand->type));
            FREE_ALL(operand, nterm);
            return NULL;
        }
    }

    nterm->type = operand->type;
    FREE_ALL(operand);
    return nterm;
}

NTerm* reduce_postfix(PushdownItem** operands, NTerm* nterm) {
    NTerm* operand = operands[0]->nterm;

    switch (operand->type) {
        case DataType_MaybeDouble:
            nterm->type = DataType_Double;
            break;
        case DataType_MaybeInt:
            nterm->type = DataType_Int;
            break;
        case DataType_MaybeBool:
            nterm->type = DataType_Bool;
            break;
        case DataType_MaybeString:
            nterm->type = DataType_String;
            break;
        default:
            expr_type_err("Cannot force unwrap value of non-optional type '%s'.", datatype_to_string(operand->type));
            FREE_ALL(operand, nterm);
            return NULL;
    }
    FREE_ALL(operand);
    return nterm;
}

NTerm* reduce_arithmetic(PushdownItem** operands, NTerm* nterm) {
    NTerm* left = operands[0]->nterm;
    Operator op = operands[1]->term->attribute.op;
    NTerm* right = operands[2]->nterm;

    if (left->is_const && right->is_const)
        nterm->is_const = true;

    if (!convert_to_same_types(left, right)) {
        expr_type_err("Invalid operands left '%s' and right '%s' operands for operation '%s'.",
                      datatype_to_string(left->type), datatype_to_string(right->type), operator_to_string(op));
        FREE_ALL(left, right, nterm);
        return NULL;
    }

    switch (op) {
        case Operator_Plus:
            // generate("+", left, right, expr.value);
            break;
        case Operator_Minus:
            // generate("-", left, right, expr.value);
            break;
        case Operator_Multiply:
            // generate("*", left, right, expr.value);
            break;
        case Operator_Divide:
            // generate("/", left, right, expr.value);
            break;
        default:
            break;
    }

    nterm->type = left->type;
    FREE_ALL(left, right);
    return nterm;
}

NTerm* reduce_logic(PushdownItem** operands, NTerm* nterm) {
    NTerm* left = operands[0]->nterm;
    NTerm* right = operands[2]->nterm;
    Operator op = operands[1]->term->attribute.op;

    if (left->is_const && right->is_const)
        nterm->is_const = true;

    if (!convert_to_same_types(left, right)) {
        expr_type_err("Invalid operands left '%s' and right '%s' operands for relation '%s'.",
                      datatype_to_string(left->type), datatype_to_string(right->type), operator_to_string(op));
        FREE_ALL(left, right, nterm);
        return NULL;
    }

    switch (left->type) {
        case DataType_Bool:
        case DataType_Int:
        case DataType_Double:
        case DataType_String:
            // generate(op, left, right, nterm.value);
            nterm->type = DataType_Bool;
            break;

        default:
            expr_type_err("Invalid operands left '%s' and right '%s' operands for relation '%s'.",
                          datatype_to_string(left->type), datatype_to_string(right->type), operator_to_string(op));
            FREE_ALL(left, right, nterm);
            return NULL;
    }

    FREE_ALL(left, right);
    return nterm;
}

NTerm* reduce_nil_coalescing(PushdownItem** operands, NTerm* nterm) {
    NTerm* left = operands[0]->nterm;
    NTerm* right = operands[2]->nterm;
    bool type_match = false;

    switch (left->type) {
        case DataType_Bool:
        case DataType_MaybeBool:
            type_match = convert_to_datatype(DataType_Bool, right);
            nterm->type = DataType_Bool;
            break;

        case DataType_Int:
        case DataType_MaybeInt:
            type_match = convert_to_datatype(DataType_Int, right);
            nterm->type = DataType_Int;
            break;

        case DataType_Double:
        case DataType_MaybeDouble:
            type_match = convert_to_datatype(DataType_Double, right);
            nterm->type = DataType_Double;
            break;

        case DataType_String:
        case DataType_MaybeString:
            type_match = convert_to_datatype(DataType_String, right);
            nterm->type = DataType_String;
            break;
        default:
            expr_type_err("Nil is not a valid type for operation ??");
            FREE_ALL(left, right, nterm);
            return NULL;
    }

    if (!type_match) {
        expr_type_err("Unexpected right operand type '%s' when left operand is of type '%s' for operation ??",
                      datatype_to_string(right->type), datatype_to_string(left->type));
        FREE_ALL(left, right, nterm);
        return NULL;
    }

    FREE_ALL(left, right);
    return nterm;
}

NTerm* reduce_args(PushdownItem** operands, NTerm* nterm) {
    NTerm* left = operands[0]->nterm;
    NTerm* right = operands[2]->nterm;

    if (stack_empty(&g_stack)) {
        syntax_err("Expected operation between operands not ','");
        FREE_ALL(left, right, nterm);
        return NULL;
    }

    StackNode* node = stack_top(&g_stack);

    if (left->name == 'E' && !left->is_named_arg) {
        if (!process_unnamed_arg(node, left)) {
            FREE_ALL(left, right, nterm);
            return NULL;
        }
    }

    if (right->name == 'E' && !right->is_named_arg) {
        if (!process_unnamed_arg(node, right)) {
            FREE_ALL(left, right, nterm);
            return NULL;
        }
    }

    nterm->name = 'L';
    FREE_ALL(left, right);
    return nterm;
}

NTerm* reduce_named_arg(PushdownItem** operands, NTerm* nterm) {
    char* arg_name = operands[0]->term->attribute.data.value.string.data;
    NTerm* arg_value = operands[2]->nterm;

    if (stack_empty(&g_stack)) {
        syntax_err("Expected operation between operands not ':'");
        FREE_ALL(arg_value, nterm);
        return NULL;
    }

    StackNode* fn_node = stack_top(&g_stack);

    MASSERT(fn_node->fn != NULL, "Fucntion symbol cannot be null in named arg rule");

    if (!process_named_arg(fn_node, arg_name, arg_value)) {
        FREE_ALL(arg_value, nterm);
        return NULL;
    }
    nterm->type = fn_node->fn->params[fn_node->processed_args - 1].type;
    nterm->is_named_arg = true;

    FREE_ALL(arg_value);
    return nterm;
}

NTerm* reduce_function(NTerm* nterm, NTerm* arg) {
    StackNode* node = stack_top(&g_stack);
    char* fn_name = node->name.data;
    FunctionSymbol* fs = node->fn;

    MASSERT(node != NULL, "Function Node cannot be null in function rule!");

    if (arg != NULL && arg->name == 'E' && !arg->is_named_arg) {
        if (!is_valid_num_of_param(node) || !process_unnamed_arg(node, arg)) {
            FREE_ALL(nterm, arg);
            return NULL;
        }
    }

    if (node->processed_args != fs->param_count) {
        fun_type_err("Mising arguments in function '%s'", fn_name);
        FREE_ALL(nterm, arg);
        return NULL;
    }

    FREE_ALL(arg);
    stack_pop(&g_stack);
    nterm->type = fs->return_value_type;
    return nterm;
}

bool convert_to_same_types(NTerm* op1, NTerm* op2) {
    if (op1->type == op2->type)
        return true;

    if (op1->is_const && op2->is_const) {
        if (op1->type == DataType_Double && op2->type == DataType_Int) {
            op2->type = DataType_Double;
            // generate(int2double, op2, , op2);
            return true;
        }
    }

    if (op1->is_const) {
        if (op1->type == DataType_Int && op2->type == DataType_Double) {
            op1->type = DataType_Double;
            // generate(int2double, op1, , op1);
            return true;
        } else if (op1->type == DataType_Double && op2->type == DataType_Int) {
            op1->type = DataType_Int;
            // generate(double2int, op1, , op1);
            return true;
        }
    }

    if (op2->is_const) {
        if (op1->type == DataType_Int && op2->type == DataType_Double) {
            op2->type = DataType_Int;
            // generate(double2int, op2, , op2);
            return true;
        } else if (op1->type == DataType_Double && op2->type == DataType_Int) {
            op2->type = DataType_Double;
            // generate(int2double, op2, , op2);
            return true;
        }
    }
    return false;
}

bool convert_to_datatype(DataType dt, NTerm* op) {
    if (dt == op->type)
        return true;

    if (op->is_const) {
        if (op->type == DataType_Int && dt == DataType_Double) {
            op->type = DataType_Double;
            // generate(int2double, op, , op);
            return true;
        } else if (op->type == DataType_Double && dt == DataType_Int) {
            op->type = DataType_Int;
            // generate(double2int, op, , op);
            return true;
        }
    }

    switch (dt) {
        case DataType_MaybeBool:
            return op->type == DataType_Bool;
        case DataType_MaybeString:
            return op->type == DataType_String;
        case DataType_MaybeInt:
            return op->type == DataType_Int;
        case DataType_MaybeDouble:
            return op->type == DataType_Double;
    }

    return false;
}

bool is_valid_num_of_param(StackNode* fn_node) {
    if (fn_node->processed_args >= fn_node->fn->param_count) {
        fun_type_err("Too many arguments for function '%s'", fn_node->name.data);
        return false;
    }
    return true;
}

bool process_unnamed_arg(StackNode* fn_node, NTerm* arg) {
    if (!is_valid_num_of_param(fn_node))
        return NULL;

    MASSERT(fn_node->processed_args < fn_node->fn->param_count,
            "Handle parameter count before unnamed argumant type check");
    MASSERT(fn_node->name.data != NULL, "Function Name cannot be null");

    FunctionParameter param = fn_node->fn->params[fn_node->processed_args];

    if (param.oname.data != NULL) {
        fun_type_err("In function '%s': The %d. argument should be named '%s', but an unnamed argument was found.",
                     fn_node->name.data, fn_node->processed_args + 1, param.oname.data);
        return false;
    }

    if (!convert_to_datatype(param.type, arg)) {
        fun_type_err("Unexpected type '%s' for %d. argument of type '%s'", datatype_to_string(arg->type),
                     fn_node->processed_args + 1, datatype_to_string(param.type));
        return false;
    }
    fn_node->processed_args++;
    return true;
}

bool process_named_arg(StackNode* fn_node, char* arg_name, NTerm* arg_value) {
    if (!is_valid_num_of_param(fn_node))
        return NULL;

    MASSERT(fn_node != NULL, "How?");
    FunctionParameter param = fn_node->fn->params[fn_node->processed_args];

    if (param.oname.data == NULL) {
        fun_type_err("In function '%s': Expected unnamed %d. argument, found named argument '%s'.", fn_node->name.data,
                     fn_node->processed_args + 1, arg_name);
        return false;
    }

    // check argument name
    if (strcmp(param.oname.data, arg_name) != 0) {
        fun_type_err("In function '%s', invalid name '%s' of the %d. argument, expected name is '%s'",
                     fn_node->name.data, arg_name, fn_node->processed_args + 1, param.oname.data);
        return false;
    }

    if (!convert_to_datatype(param.type, arg_value)) {
        fun_type_err("Unexpected type '%s' for named argument '%s'", datatype_to_string(arg_value->type), arg_name);
        return false;
    }

    fn_node->processed_args++;
    return true;
}
