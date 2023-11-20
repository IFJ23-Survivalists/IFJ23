#define POSIXLY_CORRECT
#include <iostream>
extern "C" {
#include <unistd.h>
}
#include "grammar.h"
#include "llgen.h"

struct PrgArgs {
    bool print_rules = true;
    bool print_empty = false;
    bool print_first = false;
    bool print_follow = false;
    bool print_predict = false;
    bool check_predict = true;
    bool print_lltable = true;
    bool help = false;
};

bool parse_args(int argc, char** argv, PrgArgs* args);
void usage(FILE* fs, char* prg_name);

int main(int argc, char* argv[]) {
    PrgArgs args;
    if (argc != 1) {
        if (!parse_args(argc, argv, &args)) {
            usage(stderr, argv[0]);
            return 1;
        }
        if (args.help) {
            usage(stdout, argv[0]);
            return 0;
        }
    }

    Grammar g;
    g.add_rule(NTerm::StatementList, "nn", NTerm::Statement, NTerm::StatementList);
    g.add_rule(NTerm::StatementList, NULL);
    g.add_rule(NTerm::Statement, "kanaana", Keyword_While, '(', NTerm::Expr, ')', '{', NTerm::StatementList, '}');
    g.add_rule(NTerm::Statement, "ktanattana", Keyword_Func, Token_Identifier, '(', NTerm::Params, ')', Token_ArrowRight, Token_DataType, '{', NTerm::StatementList, '}');
    g.add_rule(NTerm::Statement, "kn", Keyword_Return, NTerm::ReturnExpr);
    g.add_rule(NTerm::Statement, "knanan", Keyword_If, NTerm::IfCondition, '{', NTerm::StatementList, '}', NTerm::Else);
    g.add_rule(NTerm::Statement, "ktntn", Keyword_Let, Token_Identifier, NTerm::AssignType, Token_Equal, NTerm::Expr);
    g.add_rule(NTerm::Statement, "ktnn", Keyword_Var, Token_Identifier, NTerm::AssignType, NTerm::AssignExpr);
    g.add_rule(NTerm::Statement, "ttn", Token_Identifier, Token_Equal, NTerm::Expr);
    g.add_rule(NTerm::Statement, "n", NTerm::Expr);
    g.add_rule(NTerm::Params, "ntttn", NTerm::ArgumentLabel, Token_Identifier, Token_DoubleColon, Token_DataType, NTerm::Params_n);
    g.add_rule(NTerm::Params, NULL);
    g.add_rule(NTerm::Params_n, "tn", Token_Comma, NTerm::Params);
    g.add_rule(NTerm::Params_n, NULL);
    g.add_rule(NTerm::ArgumentLabel, "t", Token_Identifier);
    g.add_rule(NTerm::ArgumentLabel, NULL);
    g.add_rule(NTerm::ReturnExpr, "n", NTerm::Expr);
    g.add_rule(NTerm::ReturnExpr, NULL);
    g.add_rule(NTerm::IfCondition, "n", NTerm::Expr);
    g.add_rule(NTerm::IfCondition, "kn", Keyword_Let, NTerm::Expr);
    g.add_rule(NTerm::Else, "kana", Keyword_Else, '{', NTerm::StatementList, '}');
    g.add_rule(NTerm::Else, NULL);
    g.add_rule(NTerm::AssignType, "tt", Token_DoubleColon, Token_DataType);
    g.add_rule(NTerm::AssignType, NULL);
    g.add_rule(NTerm::AssignExpr, "tn", Token_Equal, NTerm::Expr);
    g.add_rule(NTerm::AssignExpr, NULL);
    g.add_rule(NTerm::Expr, "tnn", Token_Identifier, NTerm::FunctionCall, NTerm::ExprInner);
    g.add_rule(NTerm::Expr, "tn", Token_Data, NTerm::ExprInner);
    g.add_rule(NTerm::Expr, "anan", '(', NTerm::Expr, ')', NTerm::ExprInner);
    g.add_rule(NTerm::ExprInner, "tttn", Token_Whitespace, Token_Operator, Token_Whitespace, NTerm::Expr);
    g.add_rule(NTerm::ExprInner, NULL);
    g.add_rule(NTerm::FunctionCall, "ana", '(', NTerm::FunctionCallParams, ')');
    g.add_rule(NTerm::FunctionCall, NULL);
    g.add_rule(NTerm::FunctionCallParams, "ntn", NTerm::ParamName, Token_Identifier, NTerm::FunctionCallParams_n);
    g.add_rule(NTerm::FunctionCallParams, NULL);
    g.add_rule(NTerm::FunctionCallParams_n, "tn", Token_Comma, NTerm::FunctionCallParams);
    g.add_rule(NTerm::FunctionCallParams_n, NULL);
    g.add_rule(NTerm::ParamName, "tt", Token_Identifier, Token_DoubleColon);
    g.add_rule(NTerm::ParamName, NULL);

    if (args.print_rules) {
        g.print();
        std::cout << std::endl;
    }

    Empty empty(g);
    if (args.print_empty) {
        empty.print();
        std::cout << std::endl;
    }

    First first(g, empty);
    if (args.print_first) {
        first.print();
        std::cout << std::endl;
    }

    Follow follow(g, empty, first);
    if (args.print_follow) {
        follow.print();
        std::cout << std::endl;
    }

    generate_predict(g, empty, first, follow);
    if (args.print_predict) {
        print_predict(g);
        std::cout << std::endl;
    }
    if (args.check_predict) {
        check_predict(g);
        std::cout << std::endl;
    }

    if (args.print_lltable) {
        print_lltable(g);
    }
}

bool parse_args(int argc, char** argv, PrgArgs* args) {
    // Check if there are any invalid strings in cmd line
    for (int i = 1; i < argc; i++)
        if (argv[i][0] != '-')
            return false;

    *args = PrgArgs{ false, false, false, false, false, false, false, false };
    char opt;
    while ((opt = getopt(argc, argv, "hrefwpcl")) != -1) {
        switch (opt) {
            case 'h': args->help = true; return true;
            case 'r': args->print_rules = true; break;
            case 'e': args->print_empty = true; break;
            case 'f': args->print_first = true; break;
            case 'w': args->print_follow = true; break;
            case 'p': args->print_predict = true; break;
            case 'c': args->check_predict = true; break;
            case 'l': args->print_lltable = true; break;
            case '?': return false;
            default: return false;
        }
    }
    return true;
}

void usage(FILE* fs, char* prg_name) {
    fprintf(fs,
            "Usage: %s [<hrefwpcl>]\n"
            "   -h      Print this help\n"
            "   -r      Print rules\n"
            "   -e      Print empty\n"
            "   -f      Print first\n"
            "   -w      Print follow\n"
            "   -p      Print predict\n"
            "   -c      Check predict for errors\n"
            "   -l      Print LL-table\n\n"
            "By default, only options `-r`, `-c` and `-l` are used.\n"
            , prg_name);
}
