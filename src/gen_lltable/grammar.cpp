/**
 * @file grammar.cpp
 * @brief Definitions for grammar.h
 * @author Jakub Kloub, xkloub03, VUT FIT
 */
#include "grammar.h"
#include <cstdarg>
#include <cstring>
#include <iostream>

/**
 * @brief Create a rule from given tokens
 * @param lhs Left-hand side of the rule
 * @param rhsfmt Format of the right-hand side.
 *      k -> Keyword,
 *      n -> NTerm,
 *      a -> Automatic from char,
 *      t -> TokenType,
 * @pre Will set Error_Internal on error.
 * @return Created rule. This rule needs to be freed by `destroy_rule()` function.
 */
Rule create_rule(NTerm lhs, const char* rhsfmt, va_list args) {
    // Epsilon rule.
    if (rhsfmt == NULL)
        return Rule{ lhs, {} };

    Rule r;
    r.lhs = lhs;
    size_t rhs_count = strlen(rhsfmt);
    r.rhs.reserve(rhs_count);

    // Create each rhs element
    for (size_t i = 0; i < rhs_count; i++) {
        Symbol sym;
        switch (rhsfmt[i]) {
            case 'n':       // Non-terminal
                sym.is_term = false;
                sym.nterm = (NTerm)va_arg(args, int);
                break;
            case 't':       // TokenType
                sym.is_term = true;
                sym.term.tok = (TokenType)va_arg(args, int);
                break;
            case 'a':       // Automatically create a TokenType.
                sym.is_term = true;
                switch (va_arg(args, int)) {
                    case ' ': sym.term.tok = Token_Whitespace; break;
                    case '{': sym.term.tok = Token_BracketLeft; break;
                    case '}': sym.term.tok = Token_BracketRight; break;
                    case '(': sym.term.tok = Token_ParenLeft; break;
                    case ')': sym.term.tok = Token_ParenRight; break;
                    case ':': sym.term.tok = Token_DoubleColon; break;
                    case '=': sym.term.tok = Token_Equal; break;
                    case ',': sym.term.tok = Token_Comma; break;
                    default:
                        MASSERT(false, "Unknown automatic character in args.");
                        return r;
                }
                break;
            default:
                MASSERT(false, "Unknown format characer.");
                return r;
        }
        r.rhs.push_back(sym);
    }

    return r;
}

void Grammar::add_rule(NTerm lhs, const char* rhsfmt, ...) {
    // Create rule right-hand side;
    va_list args;
    va_start(args, rhsfmt);
    this->rules[lhs].push_back(create_rule(lhs, rhsfmt, args));
    va_end(args);
}

// Colors
#define CD "\033[0m"
#define CR "\033[31m"
#define CG "\033[32m"
#define CY "\033[33m"
#define CB "\033[34m"
#define CM "\033[35m"

void Grammar::print() {
    size_t n_rule = 1;
    std::cout << "Rules: " << std::endl;
    for (auto& [lhs, rules] : this->rules) {
        for (auto& rule : rules) {
            printf("    " CB "%2lu" CD ". " CG "%s" CD " -> ", n_rule++, to_string(lhs).data());
            if (rule.rhs.size() == 0)
                std::cout << CM "eps" CD;
            for (auto& sym : rule.rhs) {
                std::cout << (sym.is_term ? CY : CG) << to_string(sym) << CD " ";
            }
            std::cout << std::endl;
        }
    }
}

Grammar::Grammar() {}
Grammar::~Grammar() {}

TerminalIterator TerminalIterator::begin(int val) {
    TerminalIterator it;
    it.val = val;
    return it;
}
TerminalIterator TerminalIterator::end() {
    TerminalIterator it;
    it.val = TOK_COUNT;
    return it;
}
TerminalIterator TerminalIterator::operator+(int val) {
    TerminalIterator it;
    it.val = this->val + val;
    if (it.val > TOK_COUNT)
        it.val = TOK_COUNT;
    return it;

}
TerminalIterator TerminalIterator::operator-(int val) {
    TerminalIterator it;
    it.val = this->val + val;
    if (it.val <= 0)
        it.val = 0;
    return it;
}
TerminalIterator& TerminalIterator::operator+=(int val) {
    this->val += val;
    if (this->val > TOK_COUNT)
        this->val = TOK_COUNT;
    return (*this);
}
TerminalIterator& TerminalIterator::operator-=(int val) {
    this->val -= val;
    if (this->val < 0)
        this->val = 0;
    return (*this);
}
TerminalIterator& TerminalIterator::operator++() {
    this->val++;
    if (this->val > TOK_COUNT)
        this->val = TOK_COUNT;
    return (*this);
}
TerminalIterator& TerminalIterator::operator--() {
    this->val--;
    if (this->val < 0)
        this->val = 0;
    return (*this);
}
bool TerminalIterator::operator==(const TerminalIterator& rhs) const {
    return this->val == rhs.val;
}
bool TerminalIterator::operator!=(const TerminalIterator& rhs) const {
    return this->val != rhs.val;
}
Terminal TerminalIterator::operator*() {
    return term_from_val((TokenType)this->val);
}
Terminal TerminalIterator::operator->() {
    return term_from_val((TokenType)this->val);
}
