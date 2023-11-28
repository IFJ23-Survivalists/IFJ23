/**
 * @file expr_parser.c
 * @brief Implementation for the expr_parser.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @author Matúš Moravčík, xmorav48, FIT VUT
 * @date 27/11/2023
 */
#include "expr_parser.h"
#include <string.h>
#include "parser.h"
#include "pushdown.h"
#include "to_string.h"

const char* RULES[] = {
    "i", "(E)", "-E", "i!", "E+E", "E*E", "E>E", "E,E", "L,E", "i(L)", "i(E)", "i()", "i:E",
};

const Rule RULE_NAMES[] = {
    Rule_Identif, Rule_Paren,  Rule_Prefix,          Rule_Postfix, Rule_SumSub,  Rule_MulDiv,   Rule_Logic,
    Rule_ArgsEE,  Rule_ArgsLE, Rule_FnArgsProcessed, Rule_FnArgs,  Rule_FnEmpty, Rule_NamedArg,
};

const ComprarisonResult PRECEDENCE_TABLE[11][11] = {
    {Left, Right, Left, Right, Right, Right, Left, Right, Left, Left, Left},    /* +- */
    {Left, Left, Left, Right, Right, Right, Left, Right, Left, Left, Left},     /* * */
    {Right, Right, Left, Right, Right, Right, Left, Right, Left, Left, Left},   /* logic */
    {Left, Left, Left, Err, Right, Right, Left, Right, Left, Left, Left},       /* pre */
    {Left, Left, Left, Left, Err, Err, Left, Right, Left, Left, Left},          /* post */
    {Right, Right, Right, Right, Right, Right, Equal, Right, Right, Left, Err}, /* ( */
    {Left, Left, Left, Left, Left, Err, Left, Err, Left, Left, Left},           /* ) */
    {Left, Left, Left, Left, Equal, Equal, Left, Err, Left, Equal, Left},       /* id */
    {Right, Right, Right, Right, Right, Right, Left, Right, Left, Err, Err},    /* ,*/
    {Right, Right, Right, Right, Right, Right, Left, Right, Left, Err, Err},    /* : */
    {Right, Right, Right, Right, Right, Right, Right, Right, Err, Err, Err}};   /* $ */

bool expr_parser_begin(Data* data) {
    Pushdown* pushdown = malloc(sizeof(Pushdown));
    pushdown_init(pushdown);

    parse(pushdown, g_parser.token, NULL);

    PushdownItem item = pushdown_back(pushdown);
    NTerm* nterm = item.nterm;

    // check if pushdown is reduced to one nonterminal
    if (pushdown->size == 1 && nterm != NULL && nterm->name == 'E') {
        data->type = nterm->type;
        data->value = nterm->value;
        pushdown_destroy(pushdown);
        return true;
    }

    // syntax error occurred during parsing
    syntax_errf("Unexpected token: '%s'", token_to_string(&g_parser.token));

    pushdown_destroy(pushdown);
    return false;
}

ComprarisonResult getPrecedence(PrecedenceCat pushdownItem, PrecedenceCat inputToken) {
    return PRECEDENCE_TABLE[pushdownItem][inputToken];
}

void print_pushdown(Pushdown* pushdown) {
    printf("St: '");
    for (size_t i = 0; i < pushdown->size; i++) {
        printf("%c", pushdown->data[i].name);
    }
    printf("'");
}

int get_topmost_terminal_id(Pushdown* pushdown) {
    for (int i = pushdown->size - 1; i >= 0; i--)
        if (pushdown_at(pushdown, i).terminal)
            return i;
    return -1;
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

void parse(Pushdown* pushdown, Token token, Token* prev_token) {
    // print_pushdown(pushdown);
    int top_term_id = get_topmost_terminal_id(pushdown);
    PrecedenceCat token_prec = getTokenPrecedenceCategory(token, prev_token);

    // if there is no terminal in pushdown consider that at position -1 is '$' (PrecendeceCat_Expr_End)
    ComprarisonResult compRes = getPrecedence(
        top_term_id == -1 ? PrecendeceCat_Expr_End : char_to_precedence(pushdown_at(pushdown, top_term_id).name),
        token_prec);

    switch (compRes) {
        case Left:
            if (!reduce(pushdown))
                break;
            parse(pushdown, token, prev_token);
            break;

        case Right:
            // printf("\t >> %c\n", precedence_to_char(token_prec));
            pushdown_insert(pushdown, top_term_id + 1, create_pushdown_item(NULL, NULL));  // Rule end marker
            pushdown_push_back(pushdown, set_name(create_pushdown_item(&token, NULL), precedence_to_char(token_prec)));
            parse(pushdown, *parser_next_token(), &token);
            break;

        case Equal:
            // puts("");
            pushdown_push_back(pushdown, set_name(create_pushdown_item(&token, NULL), precedence_to_char(token_prec)));
            parse(pushdown, *parser_next_token(), &token);
            break;

        case Err:
            return;
    }
}

bool reduce(Pushdown* pushdown) {
    PushdownItem rule_operands[4];
    int idx = pushdown->size;

    // set idx to place where is topmost rule end
    while (pushdown_at(pushdown, --idx).name != '|')
        ;

    int operandCount = pushdown->size - idx - 1;

    char* rule = malloc((operandCount + 1) * sizeof(char));

    // get rule name and operands from top of the pushdown
    for (int i = 0; i < operandCount; i++) {
        PushdownItem item = pushdown_at(pushdown, idx + 1 + i);
        rule_operands[i] = item;
        rule[i] = item.name;
    }
    rule[operandCount] = '\0';

    // remove the rule from pushdown
    pushdown_reduce_size(pushdown, idx);

    Rule ruleName = get_rule(rule);
    NTerm* nterm = apply_rule(ruleName, rule_operands);

    // check if rule was applyed
    if (nterm == NULL) {
        free(rule);
        return false;
    }

    // push new non-terminal to pushdown
    pushdown_push_back(pushdown, set_name(create_pushdown_item(NULL, nterm), nterm->name));

    free(rule);
    return true;
}

Rule get_rule(char* rule) {
    // printf("\t R: %s", rule);
    for (size_t i = 0; i < RULE_COUNT; i++) {
        if (strcmp(rule, RULES[i]) == 0) {
            // printf(" -> ID: %lu \n", i);
            return RULE_NAMES[i];
        }
    }
    // printf("NO RULE\n");
    return NoRule;
}

NTerm* apply_rule(Rule rule, PushdownItem* operands) {
    NTerm* nterm = malloc(sizeof(NTerm));
    (void)operands;

    switch (rule) {
        case Rule_Identif: {
            // Token* id = operands[0].terminal;
            // // identifier
            // if (id->type == Token_Identifier) {
            //     char* id_name = id->attribute.data.value.string.data;
            //     Symtable* st = symstack_search(&g_symstack, id_name);

            //     // identifier is not defined
            //     if (st == NULL || !symtable_get_variable(st, id_name)->is_defined) {
            //         syntax_errf("Sementic error: '%s' is undefined", token_to_string(id));
            //         free(nterm);
            //         return NULL;
            //     }
            //     nterm->type = id->attribute.data_type;
            //     // generate("=", id, NULL, nterm->value);
            // }
            // // constant
            // else {
            //     nterm->type = id->attribute.data.type;
            // }
        } break;

        case Rule_Paren:
            // nterm->type = operands[1].nterm->type;
            // // nterm->value = operands[1].nterm->value;
            // free(operands[1].nterm);
            break;

        case Rule_Prefix: {
            // NTerm* expr = operands[1].nterm;  // nonterminal to be reduced
            // if (operands[0].terminal->attribute.op == Operator_Negation) {
            //     if (expr->type != DataType_Int) {  // FIXME : INT TO BOOL
            //         syntax_errf("Semantic Error: Type mismatch - Expected 'Bool', found '%s'.",
            //                     datatype_to_string(expr->type));
            //         free(nterm);
            //         free(expr);
            //         return NULL;
            //     }
            //     nterm->type = expr->type;
            // } else {
            //     if (expr->type != DataType_Int && expr->type != DataType_Double) {
            //         syntax_errf("Semantic Error: Type mismatch - Expected 'Int' or 'Double' , found '%s'.",
            //                     datatype_to_string(expr->type));
            //         free(nterm);
            //         free(expr);
            //         return NULL;
            //     }
            //     nterm->type = expr->type;
            // }
        } break;
        case Rule_Postfix: {
            // Token* id = operands[0].terminal;
            // char* id_name = id->attribute.data.value.string.data;
            // Symtable* st = symstack_search(&g_symstack, id_name);

            // // identifier
            // if (id->type == Token_Identifier) {
            //     // identifier is not defined
            //     if (st == NULL || !symtable_get_variable(st, id_name)->is_defined) {
            //         syntax_errf("Sementic error: Identifier '%s' is undefined", token_to_string(id));
            //         free(nterm);
            //         return NULL;
            //     }
            // }
            // switch (id->attribute.data_type) {
            //     case DataType_MaybeDouble:
            //         nterm->type = DataType_Double;
            //         break;
            //     case DataType_MaybeInt:
            //         nterm->type = DataType_Int;
            //         break;
            //     // case DataType_MaybeBool:
            //     // nterm->type = DataType_Bool;
            //     // break;
            //     case DataType_MaybeString:
            //         nterm->type = DataType_String;
            //         break;

            //     default:
            //         syntax_errf("Semantic Error: cannot force unwrap value of non-optional type '%s'.",
            //                     datatype_to_string(id->attribute.data_type));
            //         free(nterm);
            //         return NULL;
            // }
        } break;
        case Rule_SumSub:
            break;
        case Rule_MulDiv:
            break;
        case Rule_Logic:
            break;
        case Rule_ArgsEE:
            nterm->name = 'L';
            return nterm;
        case Rule_ArgsLE:
            nterm->name = 'L';
            return nterm;
        case Rule_FnArgsProcessed:
            break;
        case Rule_FnArgs:
            break;
        case Rule_FnEmpty:
            break;
        case Rule_NamedArg:
            break;
        case NoRule:
            return NULL;
    }

    nterm->name = 'E';  // defalut name
    return nterm;
}
