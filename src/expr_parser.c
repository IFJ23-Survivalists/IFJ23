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

#define FREE_ALL(...)                                       \
    do {                                                    \
        size_t i = 0;                                       \
        void* pta[] = {__VA_ARGS__};                        \
        for (i = 0; i < sizeof(pta) / sizeof(void*); i++) { \
            if (pta[i] != NULL) {                           \
                free(pta[i]);                               \
            }                                               \
        }                                                   \
    } while (0)

#define CHECK_ALLOCATION(ptr, ...)                               \
    do {                                                         \
        if (ptr == NULL) {                                       \
            SET_INT_ERROR(IntError_Memory, "Allocation failed"); \
            FREE_ALL(__VA_ARGS__);                               \
            return NULL;                                         \
        }                                                        \
    } while (0)

const char RULES[RULE_COUNT][MAX_RULE_LENGTH] = {
    "i", "(E)", "-E", "E!", "E+E", "E*E", "E>E", "E?E", "E,E", "L,E", "i(L)", "i(E)", "i()", "i:E",
};

const Rule RULE_NAMES[] = {
    Rule_Identif,       Rule_Paren,  Rule_Prefix, Rule_Postfix,         Rule_SumSub, Rule_MulDiv,  Rule_Logic,
    Rule_NilCoalescing, Rule_ArgsEE, Rule_ArgsLE, Rule_FnArgsProcessed, Rule_FnArgs, Rule_FnEmpty, Rule_NamedArg,
};

const char PREC_NAMES[] = {'+', '*', '>', '?', '-', '!', '(', ')', 'i', ',', ':', '$'};

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
    stack_init(&g_stack);
    pushdown_init(&g_pushdown);

    code_generation(Instruction_CreateFrame, NULL, NULL, NULL);
    parse(g_parser.token, NULL);

    // item that results from the reduction of the expression
    PushdownItem* item = pushdown_last(&g_pushdown);
    NTerm* nterm = NULL;

    if (item != NULL)
        nterm = item->nterm;

    // check if pushdown is reduced to one nonterminal else error occurred during parsing
    if (g_pushdown.first == g_pushdown.last && nterm != NULL && nterm->name == 'E') {
        data->type = nterm->type;
        data->is_nil = nterm->is_nil;

        if (nterm->code_name != NULL) {
            code_generation_raw("DEFVAR TF@res");
            code_generation_raw("MOVE TF@res %s@%s", frame_to_string(nterm->frame), nterm->code_name);
        }

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

char precedence_to_char(PrecedenceCat cat) {
    return PREC_NAMES[cat];
}

PrecedenceCat char_to_precedence(char ch) {
    for (int i = 0; i < (int)sizeof(PREC_NAMES); i++)
        if (PREC_NAMES[i] == ch)
            return i;
    MASSERT("false", "char_to_precedence: Unknown char");
    return 0;
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

char* get_unique_id() {
    static int cnt = 0;
    cnt++;
    String tmp = string_from_format("tmp%d", cnt);
    return tmp.data;
}

void parse(Token token, Token* prev_token) {
    // PushdownItem* first = g_pushdown.first;
    // while (first != NULL) {
    //     printf("%c", first->name);
    //     first = pushdown_next(first);
    // }
    // puts("");

    PushdownItem* topmost_terminal = pushdown_search_terminal(&g_pushdown);
    PrecedenceCat topmost_terminal_prec =
        topmost_terminal ? char_to_precedence(topmost_terminal->name) : PrecendeceCat_Expr_End;
    PrecedenceCat token_prec = getTokenPrecedenceCategory(token, prev_token);
    ComprarisonResult comp_res = getPrecedence(topmost_terminal_prec, token_prec);

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
                        case Token_ParenRight:
                        case Token_Identifier:
                            return PrecendeceCat_PlusMinus;

                        // binary
                        default:
                            return PrecendeceCat_Pre;
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
}

bool reduce() {
    PushdownItem* rule_operands[MAX_RULE_LENGTH];
    PushdownItem* rule_end_marker = pushdown_search_name(&g_pushdown, '|');
    int idx;

    // no rule end => no reduction possible
    if (rule_end_marker == NULL)
        return false;

    char rule[MAX_RULE_LENGTH] = {0};
    PushdownItem* item = pushdown_next(rule_end_marker);

    // get rule name and operands from top of the pushdown
    for (idx = 0; idx < MAX_RULE_LENGTH && item != NULL; idx++) {
        rule_operands[idx] = item;
        rule[idx] = item->name;
        item = pushdown_next(item);
    }

    Rule rule_name = get_rule(rule);
    NTerm* nterm = apply_rule(rule_name, rule_operands);

    // check if rule was applyed
    if (nterm == NULL) {
        if (rule_name == Rule_FnArgs)
            pushdown_remove_all_from_current(&g_pushdown, rule_end_marker);
        return false;
    }

    // remove the rule from pushdown
    pushdown_remove_all_from_current(&g_pushdown, rule_end_marker);

    // push a new non-terminal to the pushdown
    PushdownItem* nterm_item = create_pushdown_item(NULL, nterm);
    nterm_item->name = nterm->name;
    pushdown_insert_last(&g_pushdown, nterm_item);

    return true;
}

Rule get_rule(char* rule) {
    for (size_t i = 0; i < RULE_COUNT; i++) {
        size_t j;
        for (j = 0; j < MAX_RULE_LENGTH; j++) {
            if (rule[j] != RULES[i][j])
                break;
        }
        if (j == MAX_RULE_LENGTH)
            return RULE_NAMES[i];
    }
    return NoRule;
}

NTerm* apply_rule(Rule rule, PushdownItem** operands) {
    switch (rule) {
        case Rule_Identif:
            return reduce_identifier(operands[0]->term, init_nterm());
        case Rule_Paren:
            return operands[1]->nterm;
        case Rule_Prefix:
            return reduce_prefix(operands[0]->term->attribute.op, operands[1]->nterm, init_nterm());
        case Rule_Postfix:
            return reduce_postfix(operands[0]->nterm, init_nterm());
        case Rule_SumSub:
        case Rule_MulDiv:
            return reduce_arithmetic(operands[0]->nterm, operands[1]->term->attribute.op, operands[2]->nterm,
                                     init_nterm());
        case Rule_Logic:
            return reduce_logic(operands[0]->nterm, operands[1]->term->attribute.op, operands[2]->nterm, init_nterm());
        case Rule_NilCoalescing:
            return reduce_nil_coalescing(operands[0]->nterm, operands[2]->nterm, init_nterm());
        case Rule_NamedArg:
            return reduce_named_arg(operands[0]->term, operands[2]->nterm);
        case Rule_ArgsEE:
        case Rule_ArgsLE:
            return reduce_args(operands[0]->nterm, operands[2]->nterm, init_nterm());
        case Rule_FnEmpty:
            return reduce_function(operands[0]->term, NULL, init_nterm());
        case Rule_FnArgsProcessed:;
        case Rule_FnArgs:
            return reduce_function(operands[0]->term, operands[2]->nterm, init_nterm());
        default:  // NoRule:
            return NULL;
    }
}

NTerm* init_nterm() {
    NTerm* nterm = malloc(sizeof(NTerm));

    // default non-terminal attributes
    nterm->name = 'E';
    nterm->is_const = false;
    nterm->is_nil = false;
    nterm->param_name = NULL;
    nterm->frame = Frame_Temporary;
    nterm->code_name = NULL;
    return nterm;
}

NTerm* reduce_identifier(Token* id, NTerm* nterm) {
    // handle identifier
    if (id->type == Token_Identifier) {
        char* id_name = id->attribute.data.value.string.data;
        Symtable* st = symstack_search(id_name);
        VariableSymbol* vs;

        // identifier is not defined
        if (st == NULL || (vs = symtable_get_variable(st, id_name)) == NULL || !vs->is_initialized) {
            undef_var_err("Indentifier '%s' is undefined", token_to_string(id));
            FREE_ALL(nterm);
            return NULL;
        }

        nterm->type = vs->type;
        nterm->code_name = get_unique_id();
        CHECK_ALLOCATION(nterm->code_name, nterm);

        // move variable to temporary frame
        code_generation_raw("DEFVAR TF@%s", nterm->code_name);
        code_generation_raw("MOVE TF@%s %s@%s", nterm->code_name, frame_to_string(vs->code_frame), vs->code_name.data);

    }
    // handle constant
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

        nterm->code_name = var.variable.name;

        code_generation_raw("DEFVAR TF@%s", nterm->code_name);
        code_generation(Instruction_Move, &var, &symb, NULL);
    }
    return nterm;
}

NTerm* reduce_prefix(Operator op, NTerm* expr, NTerm* nterm) {
    if (expr->type == DataType_Undefined) {
        unknown_type_err("Cannot infer data type from nil");
        FREE_ALL(nterm);
        return NULL;
    }

    nterm->code_name = get_unique_id();
    CHECK_ALLOCATION(nterm->code_name, nterm);

    code_generation_raw("DEFVAR TF@%s", nterm->code_name);

    // !E
    if (op == Operator_Negation) {
        if (expr->type != DataType_Bool) {
            expr_type_err("Expected 'Bool', found '%s'.", datatype_to_string(expr->type));
            FREE_ALL(nterm->code_name, nterm);
            return NULL;
        }
        code_generation_raw("NOT TF@%s %s@%s", nterm->code_name, frame_to_string(expr->frame), expr->code_name);
    }

    // -E
    else {
        if (expr->type == DataType_Int) {
            code_generation_raw("SUB TF@%s int@0 %s@%s", nterm->code_name, frame_to_string(expr->frame),
                                expr->code_name);
        } else if (expr->type == DataType_Double) {
            code_generation_raw("SUB TF@%s float@%a %s@%s", nterm->code_name, 0.0f, frame_to_string(expr->frame),
                                expr->code_name);
        } else {
            expr_type_err("Expected 'Int' or 'Double', found '%s'.", datatype_to_string(expr->type));
            FREE_ALL(nterm->code_name, nterm);
            return NULL;
        }
    }

    nterm->type = expr->type;
    nterm->is_const = expr->is_const;
    FREE_ALL(expr->code_name, expr);
    return nterm;
}

NTerm* reduce_postfix(NTerm* expr, NTerm* nterm) {
    if (expr->type == DataType_Undefined) {
        unknown_type_err("Cannot unwrap nil value");
        FREE_ALL(nterm);
        return NULL;
    }

    switch (expr->type) {
        case DataType_MaybeDouble:
            expr->type = DataType_Double;
            break;
        case DataType_MaybeInt:
            expr->type = DataType_Int;
            break;
        case DataType_MaybeBool:
            expr->type = DataType_Bool;
            break;
        case DataType_MaybeString:
            expr->type = DataType_String;
            break;
        default:
            expr_type_err("Cannot force unwrap value of non-optional type '%s'.", datatype_to_string(expr->type));
            FREE_ALL(nterm);
            return NULL;
    }

    FREE_ALL(nterm);
    return expr;
}

NTerm* reduce_arithmetic(NTerm* left, Operator op, NTerm* right, NTerm* nterm) {
    // arithmetic operation on two constants results in constant as well
    if (left->is_const && right->is_const)
        nterm->is_const = true;
    if (!try_convert_to_same_types(left, right)) {
        FREE_ALL(nterm);
        return NULL;
    }

    switch (left->type) {
        case DataType_Bool:
        case DataType_MaybeBool:
        case DataType_MaybeDouble:
        case DataType_MaybeInt:
        case DataType_MaybeString:
            expr_type_err("Invalid operands left '%s' and right '%s'.", datatype_to_string(left->type),
                          datatype_to_string(right->type));
            FREE_ALL(nterm);
            return NULL;
        case DataType_Undefined:
            unknown_type_err("Cannot infer data type from nil");
            FREE_ALL(nterm);
            return NULL;
        default:
            break;
    }

    nterm->type = left->type;
    nterm->code_name = get_unique_id();
    CHECK_ALLOCATION(nterm->code_name, nterm);

    code_generation_raw("DEFVAR TF@%s", nterm->code_name);

    if (op == Operator_Plus && nterm->type == DataType_String) {
        code_generation_raw("CONCAT TF@%s %s@%s %s@%s", nterm->code_name, frame_to_string(left->frame), left->code_name,
                            frame_to_string(right->frame), right->code_name);
    } else {
        code_generation_raw("%s TF@%s %s@%s %s@%s", operator_to_instruction(op), nterm->code_name,
                            frame_to_string(left->frame), left->code_name, frame_to_string(right->frame),
                            right->code_name);
    }

    FREE_ALL(left->code_name, left, right->code_name, right);
    return nterm;
}

NTerm* reduce_logic(NTerm* left, Operator op, NTerm* right, NTerm* nterm) {
    if (!try_convert_to_same_types(left, right)) {
        FREE_ALL(nterm);
        return NULL;
    }
    nterm->type = DataType_Bool;
    nterm->code_name = get_unique_id();
    CHECK_ALLOCATION(nterm->code_name, nterm);

    code_generation_raw("DEFVAR TF@%s", nterm->code_name);

    switch (left->type) {
        case DataType_Bool:
            // only equality comparison and &&, || operations allowed for bool data type, not <, >, <=, >=
            if (op != Operator_DoubleEqual && op != Operator_NotEqual && op != Operator_And && op != Operator_Or) {
                expr_type_err("binary operator '%s' cannot be applied to two 'Bool' operands.", operator_to_string(op));
                FREE_ALL(nterm->code_name, nterm);
                return NULL;
            }
            code_generation_raw("%s TF@%s %s@%s %s@%s", operator_to_instruction(op), nterm->code_name,
                                frame_to_string(left->frame), left->code_name, frame_to_string(right->frame),
                                right->code_name);

            if (op == Operator_NotEqual)
                code_generation_raw("NOT TF@%s TF@%s", nterm->code_name, nterm->code_name);

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
        case DataType_MaybeBool:
        case DataType_MaybeString:
        case DataType_MaybeInt:
        case DataType_MaybeDouble:
        case DataType_Undefined:
            if (left->type == DataType_Undefined && left->is_nil ^ right->is_nil) {
                unknown_type_err("Cannot infer data type from undefined value");
                FREE_ALL(nterm->code_name, nterm);
                return NULL;
            }
            // nullable types support only == and != comparison
            if (op != Operator_DoubleEqual && op != Operator_NotEqual) {
                expr_type_err("Invalid operands left '%s' and right '%s' operands for relation '%s'.",
                              datatype_to_string(left->type), datatype_to_string(right->type), operator_to_string(op));
                FREE_ALL(nterm->code_name, nterm);
                return NULL;
            }

            code_generation_raw("EQ TF@%s %s@%s %s@%s", nterm->code_name, frame_to_string(left->frame), left->code_name,
                                frame_to_string(right->frame), right->code_name);
            if (op == Operator_NotEqual)
                code_generation_raw("NOT TF@%s TF@%s", nterm->code_name, nterm->code_name);
            break;

        default:
            break;
    }

    FREE_ALL(left->code_name, left, right->code_name, right);
    return nterm;
}

NTerm* reduce_nil_coalescing(NTerm* left, NTerm* right, NTerm* nterm) {
    bool type_match = false;

    if (right->type == DataType_Undefined) {
        expr_type_err("Right operand for ?? must not be nil");
        FREE_ALL(nterm);
        return NULL;
    }

    // convert right operand data type to the not nullable version of the left operand data type
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
        FREE_ALL(nterm);
        return NULL;
    }

    nterm->code_name = get_unique_id();
    CHECK_ALLOCATION(nterm->code_name, nterm);

    code_generation_raw("DEFVAR TF@%s", nterm->code_name);

    char* if_label = get_unique_id();
    char* else_label = get_unique_id();

    CHECK_ALLOCATION(if_label, nterm);
    CHECK_ALLOCATION(else_label, nterm);

    code_generation_raw("JUMPIFEQ %s %s@%s nil@nil", if_label, frame_to_string(left->frame), left->code_name);
    code_generation_raw("MOVE TF@%s %s@%s", nterm->code_name, frame_to_string(right->frame), right->code_name);
    code_generation_raw("JUMP %s", else_label);
    code_generation_raw("LABEL %s", if_label);
    code_generation_raw("MOVE TF@%s %s@%s", nterm->code_name, frame_to_string(right->frame), right->code_name);
    code_generation_raw("LABEL %s", else_label);

    FREE_ALL(left->code_name, left, right->code_name, right, if_label, else_label);
    return nterm;
}

NTerm* reduce_args(NTerm* left, NTerm* right, NTerm* nterm) {
    // E -> E, E => push new function node into the stack
    if (left->name == 'E') {
        stack_push(&g_stack);

        if (!insert_param(stack_top(&g_stack), left)) {
            FREE_ALL(nterm);
            return NULL;  // allocation error (realloc failed)
        }
    }

    if (!insert_param(stack_top(&g_stack), right)) {
        FREE_ALL(nterm);
        return NULL;  // allocation error (realloc failed)
    }

    if (left->name == 'L') {
        FREE_ALL(left);
    }
    nterm->name = 'L';
    return nterm;
}

NTerm* reduce_named_arg(Token* id, NTerm* arg) {
    arg->param_name = id->attribute.data.value.string.data;
    return arg;
}

NTerm* reduce_function(Token* id, NTerm* arg, NTerm* nterm) {
    // handle single parameter function
    if (arg != NULL && arg->name == 'E') {
        stack_push(&g_stack);
        if (!insert_param(stack_top(&g_stack), arg)) {
            FREE_ALL(nterm);
            return NULL;  // allocation error (realloc failed)
        }
        arg = NULL;  // avoid double free
    }
    // no parameter function
    else if (arg == NULL)
        stack_push(&g_stack);

    String fn_name = id->attribute.data.value.string;
    StackNode* top_fn = stack_top(&g_stack);  // the topmost function to be called

    // handle function call on any data type e.g. 12() or true()
    if (id->type == Token_Data) {
        syntax_err("'%s is not callable", tokentype_to_string(id->type));
        FREE_ALL(nterm);
        return NULL;
    }

    // handle `write` function call
    if (strcmp(fn_name.data, "write") == 0) {
        for (int i = 0; i < top_fn->param_count; i++) {
            NTerm* param = top_fn->param[i];

            // error if named argument provided
            if (param->param_name != NULL) {
                fun_type_err("Invalid argument for write function");
                FREE_ALL(nterm);
                return NULL;
            }

            code_generation_raw("WRITE %s@%s", frame_to_string(param->frame), param->code_name);
        }
        stack_pop(&g_stack);
        nterm->type = DataType_Undefined;
        FREE_ALL(arg);
        return nterm;
    }

    FunctionSymbol* expected_function = get_fn_symbol(fn_name);

    if (!expected_function) {
        FREE_ALL(nterm);
        return NULL;  // undefined function error
    }

    // check number of arguments
    if (top_fn != NULL && expected_function->param_count != top_fn->param_count) {
        fun_type_err("Inavalid number of arguments in function '%s', expected %d, found %d.", fn_name.data,
                     expected_function->param_count, top_fn->param_count);
        FREE_ALL(nterm);
        return NULL;
    }

    code_generation_raw("PUSHFRAME");
    code_generation_raw("CREATEFRAME");

    // compare arguments names and types
    for (int i = 0; i < top_fn->param_count; i++) {
        FunctionParameter expected_param = expected_function->params[i];
        NTerm* provided_arg = top_fn->param[i];

        // check if both parameters are named or unnamed
        if (expected_param.is_named ^ (provided_arg->param_name != NULL)) {
            fun_type_err("Unexpected name for %d. argument in function '%s'", i + 1, fn_name.data);
            FREE_ALL(nterm);
            return NULL;
        }
        // check if both are named or unnamed
        if (provided_arg->param_name && strcmp(expected_param.oname.data, provided_arg->param_name) != 0) {
            fun_type_err("Unexpected name for %d. argument in function '%s'", i + 1, fn_name.data);
            FREE_ALL(nterm);
            return NULL;
        }

        // check type and name of the arguments
        if (!try_convert_to_datatype(expected_param.type, provided_arg, true)) {
            fun_type_err("Unexpected type '%s' for %d. argument in function '%s'",
                         datatype_to_string(provided_arg->type), i + 1, fn_name.data);
            FREE_ALL(nterm);
            return NULL;
        }

        code_generation_raw("DEFVAR TF@%s", expected_param.code_name.data);
        code_generation_raw("MOVE TF@%s LF@%s", expected_param.code_name.data, provided_arg->code_name);
    }

    stack_pop(&g_stack);

    nterm->type = expected_function->return_value_type;
    nterm->code_name = get_unique_id();
    CHECK_ALLOCATION(nterm->code_name, nterm);

    code_generation_raw("CALL %s", expected_function->code_name.data);
    code_generation_raw("DEFVAR LF@%s", nterm->code_name);
    code_generation_raw("MOVE LF@%s TF@ret", nterm->code_name);
    code_generation_raw("POPFRAME");

    FREE_ALL(arg);
    return nterm;
}

bool try_convert_to_same_types(NTerm* op1, NTerm* op2) {
    if (op1->type == op2->type)
        return true;

    // if both operands are constants and one is of type Int and the other one Double make sure the resulting value is
    // is of data type double
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
        } else if (op1->is_nil) {
            switch (op2->type) {
                case DataType_MaybeBool:
                case DataType_MaybeString:
                case DataType_MaybeInt:
                case DataType_MaybeDouble:
                    op1->type = op2->type;
                    return true;
                default:
                    unknown_type_err("Cannot infer data type from nil");
                    return false;
            }
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
        } else if (op2->is_nil) {
            switch (op1->type) {
                case DataType_MaybeBool:
                case DataType_MaybeString:
                case DataType_MaybeInt:
                case DataType_MaybeDouble:
                    op2->type = op1->type;
                    return true;
                    break;
                default:
                    unknown_type_err("Cannot infer data type from nil");
                    return false;
            }
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

FunctionSymbol* get_fn_symbol(String fn_name) {
    Symtable* sym = symstack_search(fn_name.data);

    if (sym == NULL) {
        undef_fun_err("Undefined function '%s'.", fn_name.data);
        return NULL;
    }

    FunctionSymbol* fs = symtable_get_function(sym, fn_name.data);
    if (fs == NULL) {
        undef_fun_err("'%s' is not a function", fn_name.data);
        return NULL;
    }
    return fs;
}
