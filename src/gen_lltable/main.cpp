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
    g.add_rule(NTerm_StatementList, "nnn", NTerm_Statement, NTerm_StatementSeparator, NTerm_StatementList);
    g.add_rule(NTerm_StatementSeparator, "t", Token_Whitespace);     // Whitespace with `has_eol` attribute.
    g.add_rule(NTerm_StatementSeparator, "t", Token_BracketRight);
    g.add_rule(NTerm_StatementSeparator, "t", Token_EOF);
    g.add_rule(NTerm_StatementList, NULL);

    g.add_rule(NTerm_Statement, "tnanan", Token_While, NTerm_Expr, '{', NTerm_StatementList, '}', NTerm_StatementList);
    g.add_rule(NTerm_Statement, "ttanananan", Token_Func, Token_Identifier, '(', NTerm_Params, ')', NTerm_FuncReturnType, '{', NTerm_StatementList, '}', NTerm_StatementList);
    g.add_rule(NTerm_Statement, "tn", Token_Return, NTerm_ReturnExpr);
    g.add_rule(NTerm_Statement, "tn", Token_If, NTerm_IfStatement);
    g.add_rule(NTerm_Statement, "ttntn", Token_Let, Token_Identifier, NTerm_AssignType, Token_Equal, NTerm_Expr);
    g.add_rule(NTerm_Statement, "ttnn", Token_Var, Token_Identifier, NTerm_AssignType, NTerm_AssignExpr);
    g.add_rule(NTerm_Statement, "ttn", Token_Identifier, Token_Equal, NTerm_Expr);
    g.add_rule(NTerm_Statement, "n", NTerm_Expr);

    g.add_rule(NTerm_FuncReturnType, "tt", Token_ArrowRight, Token_DataType);
    g.add_rule(NTerm_FuncReturnType, NULL);
    g.add_rule(NTerm_IfStatement, "nanan", NTerm_IfCondition, '{', NTerm_StatementList, '}', NTerm_Else);
    g.add_rule(NTerm_Params, "ttttn", Token_Identifier, Token_Identifier, Token_DoubleColon, Token_DataType, NTerm_Params_n);
    g.add_rule(NTerm_Params, NULL);
    g.add_rule(NTerm_Params_n, "tn", Token_Comma, NTerm_Params);
    g.add_rule(NTerm_Params_n, NULL);
    g.add_rule(NTerm_ReturnExpr, "n", NTerm_Expr);
    g.add_rule(NTerm_ReturnExpr, NULL);
    g.add_rule(NTerm_IfCondition, "n", NTerm_Expr);
    g.add_rule(NTerm_IfCondition, "tn", Token_Let, NTerm_Expr);

    g.add_rule(NTerm_Else, "tn", Token_Else, NTerm_ElseIf);
    g.add_rule(NTerm_Else, "n", NTerm_StatementList);
    g.add_rule(NTerm_ElseIf, "anan", '{', NTerm_StatementList, '}', NTerm_Else);
    g.add_rule(NTerm_ElseIf, "tn", Token_If, NTerm_IfStatement);

    g.add_rule(NTerm_AssignType, "tt", Token_DoubleColon, Token_DataType);
    g.add_rule(NTerm_AssignType, NULL);
    g.add_rule(NTerm_AssignExpr, "tn", Token_Equal, NTerm_Expr);
    g.add_rule(NTerm_AssignExpr, NULL);

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
