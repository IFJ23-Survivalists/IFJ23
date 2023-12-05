/**
 * @file expr_parser.c
 * @brief Implementation for the expr_parser.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @author Matúš Moravčík, xmorav48, FIT VUT
 * @date 27/11/2023
 */
#include "expr_parser.h"
#include <string.h>
#include "codegen.h"
#include "function_stack.h"
#include "parser.h"
#include "pushdown.h"
#include "symtable.h"
#include "to_string.h"

// Pool* g_pool = NULL;

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
    {Left, Right, Left, Left, Right, Right, Right, Left, Right, Left, Left, Left},    /* +- */
    {Left, Left, Left, Left, Right, Right, Right, Left, Right, Left, Left, Left},     /* * */
    {Right, Right, Err, Left, Right, Right, Right, Left, Right, Left, Left, Left},    /* logic ==, <, >... */
    {Right, Right, Right, Right, Right, Right, Right, Left, Right, Left, Err, Left},  /* ?? */
    {Left, Left, Left, Left, Err, Right, Right, Left, Right, Left, Left, Left},       /* pre */
    {Left, Left, Left, Left, Left, Err, Err, Left, Right, Left, Left, Left},          /* post */
    {Right, Right, Right, Right, Right, Right, Right, Equal, Right, Right, Err, Err}, /* ( */
    {Left, Left, Left, Left, Left, Left, Err, Left, Err, Left, Left, Left},           /* ) */
    {Left, Left, Left, Left, Left, Left, Equal, Left, Err, Left, Equal, Left},        /* id */
    {Right, Right, Right, Right, Right, Right, Right, Left, Right, Left, Err, Err},   /* ,*/
    {Right, Right, Right, Right, Right, Right, Right, Left, Right, Left, Err, Err},   /* : */
    {Right, Right, Right, Right, Right, Right, Right, Err, Right, Err, Err, Err}};    /* $ */

Stack g_stack;
Pushdown g_pushdown;

bool expr_parser_begin(Data* data) {
    NTerm* nterm = NULL;

    stack_init(&g_stack);
    pushdown_init(&g_pushdown);

    code_generation(Instruction_CreateFrame, NULL, NULL, NULL);
    parse(g_parser.token, NULL);

    // item that results from the reduction of the expression
    PushdownItem* item = pushdown_last(&g_pushdown);

    if (item != NULL)
        nterm = item->nterm;

    // check if pushdown is reduced to one nonterminal else error occurred during parsing
    if (g_pushdown.first == g_pushdown.last && nterm != NULL && nterm->name == 'E') {
        data->type = nterm->type;
        data->is_nil = nterm->is_nil;

        code_generation_raw("DEFVAR TF@res");
        code_generation_raw("MOVE TF@res %s@%s", frame_to_string(nterm->frame), nterm->code_name);

        stack_free(&g_stack);
        pushdown_free(&g_pushdown);
        return true;
    }

    // check if a semantic error occured during parsing otherwise a syntax error occured
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

char* operator_to_instruction(Operator op) {
    switch (op) {
        case Operator_And:
            return "AND";
        case Operator_Or:
            return "OR";
        case Operator_DoubleEqual:
        case Operator_NotEqual:
            return "EQ";
        case Operator_LessThan:
        case Operator_LessOrEqual:
            return "LT";
        case Operator_MoreThan:
        case Operator_MoreOrEqual:
            return "GT";
        case Operator_Plus:
            return "ADD";
        case Operator_Minus:
            return "SUB";
        case Operator_Multiply:
            return "MUL";
        case Operator_Divide:
            return "DIV";
        default:
            SET_INT_ERROR(IntError_InvalidArgument, "Invalid operator");
            return NULL;
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
        case Left:  // apply rule
            if (reduce())
                parse(token, prev_token);
            break;

        case Right: {  // insert token to pushdown with rule end marker
            PushdownItem* rule_end_marker = create_pushdown_item(NULL, NULL);
            PushdownItem* term = create_pushdown_item(&token, NULL);
            term->name = precedence_to_char(token_prec);

            pushdown_insert_after(&g_pushdown, topmost_terminal, rule_end_marker);
            pushdown_insert_last(&g_pushdown, term);

            parse(*parser_next_token(), &token);
        } break;

        case Equal: {  // insert token to pushdown
            PushdownItem* term = create_pushdown_item(&token, NULL);
            term->name = precedence_to_char(token_prec);
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
    nterm->is_nil = false;
    nterm->param_name = NULL;
    nterm->frame = Frame_Temporary;

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
            return reduce_function(nterm, operands[0]->term, NULL);
        case Rule_FnArgsProcessed:;
        case Rule_FnArgs:
            return reduce_function(nterm, operands[0]->term, operands[2]->nterm);
        case Rule_NamedArg:
            return reduce_named_arg(operands, nterm);
        case NoRule:
            FREE_ALL(nterm);
            return NULL;
    }
    return nterm;
}

char* get_unique_id() {
    static int cnt = 0;
    cnt++;
    String tmp = string_from_format("tmp%d", cnt);
    return tmp.data;
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
        nterm->frame = Frame_Temporary;
        nterm->code_name = get_unique_id();

        code_generation_raw("DEFVAR TF@%s", nterm->code_name);
        code_generation_raw("MOVE TF@%s %s@%s", nterm->code_name, frame_to_string(vs->code_frame), vs->code_name.data);

    }
    // constant
    else {
        Operand symb;
        Operand var;

        if (id->attribute.data.is_nil) {
            nterm->type = DataType_Undefined;
            nterm->is_nil = true;
            symb.symbol.constant.is_nil = true;
        } else
            nterm->type = id->attribute.data.type;

        symb.symbol.type = SymbolType_Constant;
        symb.symbol.constant.type = nterm->type;
        nterm->is_const = true;

        var.variable.frame = Frame_Temporary;
        var.variable.name = get_unique_id();

        if (var.variable.name == NULL) {
            return NULL;  // allocation error
        }

        switch (nterm->type) {
            case DataType_Bool:
                symb.symbol.constant.value.is_true = id->attribute.data.value.is_true;
                break;
            case DataType_Int:
                symb.symbol.constant.value.number = id->attribute.data.value.number;
                break;
            case DataType_Double:
                symb.symbol.constant.value.number_double = id->attribute.data.value.number_double;
                break;
            case DataType_String:
                symb.symbol.constant.value.string = id->attribute.data.value.string;
                break;
            default:
                symb.symbol.constant.is_nil = true;
                break;
        }

        nterm->frame = Frame_Temporary;
        nterm->code_name = var.variable.name;

        code_generation_raw("DEFVAR TF@%s", nterm->code_name);
        code_generation(Instruction_Move, &var, &symb, NULL);
    }
    return nterm;
}

NTerm* reduce_parenthesis(PushdownItem** operands, NTerm* nterm) {
    NTerm* expr = operands[1]->nterm;
    nterm->type = expr->type;
    nterm->is_const = expr->is_const;
    nterm->is_nil = expr->is_nil;
    nterm->code_name = expr->code_name;
    nterm->frame = expr->frame;
    FREE_ALL(expr);
    return nterm;
}

NTerm* reduce_prefix(PushdownItem** operands, NTerm* nterm) {
    NTerm* operand = operands[1]->nterm;

    if (operand->type == DataType_Undefined) {
        unknown_type_err("Cannot infer data type from nil");
        FREE_ALL(operand, nterm);
        return NULL;
    }

    nterm->code_name = get_unique_id();
    code_generation_raw("DEFVAR TF@%s", nterm->code_name);

    // !E
    if (operands[0]->term->attribute.op == Operator_Negation) {
        if (operand->type != DataType_Bool) {
            expr_type_err("Expected 'Bool', found '%s'.", datatype_to_string(operand->type));
            FREE_ALL(operand, nterm->code_name, nterm);
            return NULL;
        }
        code_generation_raw("NOT TF@%s %s@%s", nterm->code_name, frame_to_string(operand->frame), operand->code_name);
    }

    // -E
    else {
        if (operand->type == DataType_Int) {
            code_generation_raw("SUB TF@%s int@0 %s@%s", nterm->code_name, frame_to_string(operand->frame),
                                operand->code_name);
        } else if (operand->type == DataType_Double) {
            code_generation_raw("SUB TF@%s float@%a %s@%s", nterm->code_name, 0.0f, frame_to_string(operand->frame),
                                operand->code_name);
        } else {
            expr_type_err("Expected 'Int' or 'Double', found '%s'.", datatype_to_string(operand->type));
            FREE_ALL(operand, nterm->code_name, nterm);
            return NULL;
        }
    }

    nterm->type = operand->type;
    nterm->is_const = operand->is_const;
    FREE_ALL(operand);
    return nterm;
}

NTerm* reduce_postfix(PushdownItem** operands, NTerm* nterm) {
    NTerm* operand = operands[0]->nterm;

    if (operand->type == DataType_Undefined) {
        unknown_type_err("Cannot infer data type from nil");
        FREE_ALL(operand, nterm);
        return NULL;
    }

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

    nterm->code_name = operand->code_name;
    nterm->is_const = operand->is_const;
    nterm->frame = operand->frame;

    FREE_ALL(operand);
    return nterm;
}

NTerm* reduce_arithmetic(PushdownItem** operands, NTerm* nterm) {
    NTerm* left = operands[0]->nterm;
    Operator op = operands[1]->term->attribute.op;
    NTerm* right = operands[2]->nterm;

    // arithmetic operation on two constants is constant as well
    if (left->is_const && right->is_const)
        nterm->is_const = true;

    if (!try_convert_to_same_types(left, right)) {
        FREE_ALL(left, right, nterm);
        return NULL;
    }

    if (left->type == DataType_Bool) {
        expr_type_err("Cannot add two Bools");
        FREE_ALL(left, right, nterm);
        return NULL;
    }

    nterm->type = left->type;
    nterm->code_name = get_unique_id();

    code_generation_raw("DEFVAR TF@%s", nterm->code_name);

    if (op == Operator_Plus && nterm->type == DataType_String) {
        code_generation_raw("CONCAT TF@%s %s@%s %s@%s", nterm->code_name, frame_to_string(left->frame), left->code_name,
                            frame_to_string(right->frame), right->code_name);
    } else {
        code_generation_raw("%s TF@%s %s@%s %s@%s", operator_to_instruction(op), nterm->code_name,
                            frame_to_string(left->frame), left->code_name, frame_to_string(right->frame),
                            right->code_name);
    }

    FREE_ALL(left, right);
    return nterm;
}

NTerm* reduce_logic(PushdownItem** operands, NTerm* nterm) {
    NTerm* left = operands[0]->nterm;
    NTerm* right = operands[2]->nterm;
    Operator op = operands[1]->term->attribute.op;

    if (!try_convert_to_same_types(left, right)) {
        FREE_ALL(left, right, nterm);
        return NULL;
    }

    nterm->type = DataType_Bool;
    nterm->code_name = get_unique_id();

    code_generation_raw("DEFVAR TF@%s", nterm->code_name);

    switch (left->type) {
        case DataType_Bool:
            // only equality comparison and &&, || operations allowed for bool data type, not <, >, <=, >=
            if (op != Operator_DoubleEqual && op != Operator_NotEqual && op != Operator_And && op != Operator_Or) {
                expr_type_err("binary operator '%s' cannot be applied to two 'Bool' operands.", operator_to_string(op));
                FREE_ALL(left, right, nterm);
                return NULL;
            }
            code_generation_raw("%s TF@%s %s@%s %s@%s", operator_to_instruction(op), nterm->code_name,
                                frame_to_string(left->frame), left->code_name, frame_to_string(right->frame),
                                right->code_name);

            if (op == Operator_NotEqual) {
                code_generation_raw("NOT TF@%s TF@%s", nterm->code_name, nterm->code_name);
            }
            break;
        case DataType_Int:
        case DataType_Double:
        case DataType_String:
            code_generation_raw("%s TF@%s %s@%s %s@%s", operator_to_instruction(op), nterm->code_name,
                                frame_to_string(left->frame), left->code_name, frame_to_string(right->frame),
                                right->code_name);
            switch (op) {
                case Operator_LessOrEqual:
                case Operator_MoreOrEqual: {
                    char* tmp = get_unique_id();
                    code_generation_raw("EQ TF@%s %s@%s %s@%s", tmp, frame_to_string(left->frame), left->code_name,
                                        frame_to_string(right->frame), right->code_name);

                    code_generation_raw("OR TF@%s TF@%s TF@%s", nterm->code_name, nterm->code_name, tmp);
                    FREE_ALL(tmp);
                    break;
                } break;
                case Operator_NotEqual:
                    code_generation_raw("NOT TF@%s TF@%s", nterm->code_name, nterm->code_name);
                    break;
                default:
                    break;
            }
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

    if (right->type == DataType_Undefined) {
        expr_type_err("Right operand for ?? must not be nil");
        FREE_ALL(left, right, nterm);
        return NULL;
    }

    switch (left->type) {
        case DataType_Bool:
        case DataType_MaybeBool:
            type_match = try_convert_to_datatype(DataType_Bool, right, false);
            nterm->type = DataType_Bool;
            break;

        case DataType_Int:
        case DataType_MaybeInt:
            type_match = try_convert_to_datatype(DataType_Int, right, false);
            nterm->type = DataType_Int;
            break;

        case DataType_Double:
        case DataType_MaybeDouble:
            type_match = try_convert_to_datatype(DataType_Double, right, false);
            nterm->type = DataType_Double;
            break;

        case DataType_String:
        case DataType_MaybeString:
            type_match = try_convert_to_datatype(DataType_String, right, false);
            nterm->type = DataType_String;
            break;
        case DataType_Undefined:
            nterm->type = right->type;
            nterm->is_const = right->is_const;
            type_match = true;
    }

    if (!type_match) {
        expr_type_err("Unexpected right operand type '%s' when left operand is of type '%s' for operation ??",
                      datatype_to_string(right->type), datatype_to_string(left->type));
        FREE_ALL(left, right, nterm);
        return NULL;
    }

    nterm->code_name = get_unique_id();
    code_generation_raw("DEFVAR TF@%s", nterm->code_name);

    char* if_label = get_unique_id();
    char* else_label = get_unique_id();

    code_generation_raw("JUMPIFEQ %s %s@%s nil@nil", if_label, frame_to_string(left->frame), left->code_name);
    code_generation_raw("MOVE TF@%s %s@%s", nterm->code_name, frame_to_string(right->frame), right->code_name);
    code_generation_raw("JUMP %s", else_label);
    code_generation_raw("LABEL %s", if_label);
    code_generation_raw("MOVE TF@%s %s@%s", nterm->code_name, frame_to_string(right->frame), right->code_name);
    code_generation_raw("LABEL %s", else_label);

    FREE_ALL(left, right, if_label, else_label);
    return nterm;
}

NTerm* reduce_args(PushdownItem** operands, NTerm* nterm) {
    NTerm* left = operands[0]->nterm;
    NTerm* right = operands[2]->nterm;

    // E -> E, E => push new function stack node
    if (left->name == 'E') {
        stack_push(&g_stack);

        if (!insert_param(stack_top(&g_stack), left)) {
            FREE_ALL(left, right, nterm);
            return NULL;  // allocation error
        }
    }

    if (!insert_param(stack_top(&g_stack), right)) {
        FREE_ALL(left, right, nterm);
        return NULL;  // allocation error
    }

    nterm->name = 'L';

    return nterm;
}

NTerm* reduce_named_arg(PushdownItem** operands, NTerm* nterm) {
    NTerm* arg_value = operands[2]->nterm;

    nterm->param_name = operands[0]->term->attribute.data.value.string.data;
    nterm->type = operands[2]->nterm->type;
    nterm->is_const = arg_value->is_const;
    nterm->is_nil = arg_value->is_nil;
    nterm->frame = arg_value->frame;
    nterm->code_name = arg_value->code_name;

    FREE_ALL(arg_value);
    return nterm;
}

NTerm* reduce_function(NTerm* nterm, Token* id, NTerm* arg) {
    if (id->type == Token_Data) {
        syntax_err("'%s is not callable", tokentype_to_string(id->type));
        FREE_ALL(nterm, arg);
        return NULL;
    }

    // handle single argument function
    if (arg != NULL && arg->name == 'E') {
        stack_push(&g_stack);
        if (!insert_param(stack_top(&g_stack), arg)) {
            FREE_ALL(arg, nterm);
            return NULL;  // allocation error
        }
    }
    // no argument function
    else if (arg == NULL) {
        stack_push(&g_stack);
    }

    String fn_name = id->attribute.data.value.string;
    StackNode* node = stack_top(&g_stack);
    FunctionSymbol* expected_function = get_fn_symbol(fn_name);

    if (!expected_function) {
        FREE_ALL(arg, nterm);
        return NULL;  // undefined function
    }

    // check number of parameters
    if (node != NULL && expected_function->param_count != node->param_count) {
        fun_type_err("Inavalid number of arguments in function '%s', expected %d, found %d.", fn_name.data,
                     expected_function->param_count, node->param_count);
        FREE_ALL(nterm, arg);
        return NULL;
    }

    code_generation_raw("PUSHFRAME");
    code_generation_raw("CREATEFRAME");

    // compare arguments names and types
    for (int i = 0; i < node->param_count; i++) {
        FunctionParameter expected_param = expected_function->params[i];
        NTerm* found_param = node->param[i];

        // check if both are named or unnamed
        if ((expected_param.is_named ^ (found_param->param_name != NULL))) {
            fun_type_err("Unexpected name for %d. argument in function '%s'", i, fn_name.data);
            FREE_ALL(arg, nterm);
            return NULL;
        }
        if (found_param->param_name && strcmp(expected_param.oname.data, found_param->param_name) != 0) {
            fun_type_err("Unexpected name for %d. argument in function '%s'", i, fn_name.data);
            FREE_ALL(arg, nterm);
            return NULL;
        }

        // check type and name of the parameters
        if (!try_convert_to_datatype(expected_param.type, found_param, true)) {
            fun_type_err("Unexpected type '%s' for %d. argument in function '%s'",
                         datatype_to_string(found_param->type), i, fn_name.data);
            FREE_ALL(arg, nterm);
            return NULL;
        }

        code_generation_raw("DEFVAR TF@%s", expected_param.code_name.data);
        code_generation_raw("MOVE TF@%s LF@%s", expected_param.code_name.data, found_param->code_name);

        FREE_ALL(found_param);
    }

    if (arg->name == 'L') {
        FREE_ALL(arg);
    }

    FREE_ALL(node->param);

    stack_pop(&g_stack);

    nterm->type = expected_function->return_value_type;
    nterm->code_name = get_unique_id();
    nterm->frame = Frame_Temporary;

    code_generation_raw("CALL %s", expected_function->code_name.data);

    code_generation_raw("DEFVAR LF@%s", nterm->code_name);
    code_generation_raw("MOVE LF@%s TF@ret", nterm->code_name);

    code_generation_raw("POPFRAME");

    return nterm;
}

bool try_convert_to_same_types(NTerm* op1, NTerm* op2) {
    if (op1->type == DataType_Undefined || op2->type == DataType_Undefined) {
        unknown_type_err("Cannot infer data type from nil");
        return false;
    }

    if (op1->type == op2->type)
        return true;

    if (op1->is_const && op2->is_const) {
        if (op1->type == DataType_Double && op2->type == DataType_Int) {
            op2->type = DataType_Double;
            code_generation_raw("INT2FLOAT %s@%s %s@%s", frame_to_string(op2->frame), op2->code_name,
                                frame_to_string(op2->frame), op2->code_name);
            return true;
        }
    }

    if (op1->is_const) {
        if (op1->type == DataType_Int && op2->type == DataType_Double) {
            op1->type = DataType_Double;
            code_generation_raw("INT2FLOAT %s@%s %s@%s", frame_to_string(op1->frame), op1->code_name,
                                frame_to_string(op1->frame), op1->code_name);
            return true;
        } else if (op1->type == DataType_Double && op2->type == DataType_Int) {
            op1->type = DataType_Int;
            code_generation_raw("FLOAT2INT %s@%s %s@%s", frame_to_string(op1->frame), op1->code_name,
                                frame_to_string(op1->frame), op1->code_name);

            return true;
        }
    }

    if (op2->is_const) {
        if (op1->type == DataType_Int && op2->type == DataType_Double) {
            op2->type = DataType_Int;
            code_generation_raw("FLOAT2INT %s@%s %s@%s", frame_to_string(op2->frame), op2->code_name,
                                frame_to_string(op2->frame), op2->code_name);
            return true;

        } else if (op1->type == DataType_Double && op2->type == DataType_Int) {
            op2->type = DataType_Double;
            code_generation_raw("INT2FLOAT %s@%s %s@%s", frame_to_string(op2->frame), op2->code_name,
                                frame_to_string(op2->frame), op2->code_name);

            return true;
        }
    }

    expr_type_err("Invalid operands left '%s' and right '%s'.", datatype_to_string(op1->type),
                  datatype_to_string(op2->type));

    return false;
}

bool try_convert_to_datatype(DataType dt, NTerm* operand, bool allow_nil) {
    if (dt == operand->type)
        return true;

    if (operand->is_const) {
        if (operand->type == DataType_Int && dt == DataType_Double) {
            operand->type = DataType_Double;
            code_generation_raw("INT2FLOAT TF@%s TF@%s", operand->code_name, operand->code_name);
            return true;
        } else if (operand->type == DataType_Double && dt == DataType_Int) {
            operand->type = DataType_Int;
            code_generation_raw("FLOAT2INT TF@%s TF@%s", operand->code_name, operand->code_name);
            return true;
        }
    }

    switch (dt) {
        case DataType_MaybeBool:
            return (operand->type == DataType_Bool || (allow_nil && operand->type == DataType_Undefined));
        case DataType_MaybeString:
            return (operand->type == DataType_String || (allow_nil && operand->type == DataType_Undefined));
        case DataType_MaybeInt:
            return (operand->type == DataType_Int || (allow_nil && operand->type == DataType_Undefined));
        case DataType_MaybeDouble:
            return (operand->type == DataType_Double || (allow_nil && operand->type == DataType_Undefined));
        default:
            false;
    }

    return false;
}
