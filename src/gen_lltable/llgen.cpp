#include "llgen.h"
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <optional>

// Colors
#define CD "\033[0m"
#define CR "\033[31m"
#define CG "\033[32m"
#define CY "\033[33m"
#define CB "\033[34m"
#define CM "\033[35m"

Empty::Empty(Grammar& g) {
    for (int nterm = 0; nterm < (int)NTerm::count; nterm++) {
        for (auto& rule : g.rules[NTerm(nterm)]) {
            if (rule.eps())
                empty[NTerm(nterm)] = true;
            else
                empty[NTerm(nterm)] |= false;
        }
    }

    bool changed;
    do {
        changed = false;
        for (auto& [nterm, rules] : g.rules) {
            if (empty[nterm] == true)
                continue;
            for (auto& rule : rules) {
                empty[nterm] = true;
                for (auto& s : rule.rhs)
                    if (!(*this)[s]) {
                        empty[nterm] = false;
                        break;
                    }
                if (empty[nterm]) {
                    changed = true;
                    break;
                }
            }
        }
    } while (changed);
}

bool Empty::get(const std::vector<Symbol>::iterator& begin,
                const std::vector<Symbol>::iterator& end) {
    for (size_t k = 0; begin + k != end; k++)
        if (!(*this)[*(begin + k)])
            return false;
    return true;
}

void Empty::print() {
    for (auto& [nterm, b] : empty) {
        printf("Empty(%s%s%s) = %s%s%s\n", CG, to_string(nterm).data(), CD, CM, b ? "true" : "false", CD);
    }
}

First::First(Grammar& g, Empty& empty) : empty(empty) {
    bool changed;
    do {
        changed = false;
        for (auto& [nterm, rules] : g.rules) {
            // For all rules of type A -> X1 X2 ... Xk-1 Xk ... Xn
            for (auto& rule : rules) {
                if (rule.eps())
                    continue;
                const auto& A = rule.lhs;
                size_t init_size = first[A].size();
                // Add all symbols from First(X1) into First(A)
                const Symbol& X1 = rule.rhs[0];
                if (X1.is_term)
                    first[A].insert(X1.term);
                else
                    first[A].insert(first[X1.nterm].begin(), first[X1.nterm].end());

                size_t k = 0;
                for (k = 0; k <= rule.rhs.size() && empty[rule.rhs[k]]; k++);
                if (k != 0) {
                    if (rule.rhs[k].is_term)
                        first[A].insert(rule.rhs[k].term);
                    else
                        first[A].insert(first[rule.rhs[k].nterm].begin(), first[rule.rhs[k].nterm].end());
                }

                if (first[A].size() != init_size)
                    changed = true;
            }
        }
    } while (changed);
}

std::unordered_set<Terminal> First::get(const std::vector<Symbol>::iterator& begin,
                                        const std::vector<Symbol>::iterator& end) {
    // First(eps) = {}
    if (begin == end)
        return {};

    // First(X1X2..Xn) := First(X1)
    std::unordered_set<Terminal> set = (*this)[*begin];

    // Find first Xk, where Empty(Xk) = false
    size_t k;
    for (k = 0; begin + k != end && empty[*(begin + k)]; k++)
        ;

    if (k != 0 && begin + k != end) {
        // Add all symbols from First(Xk) into First(X1X2..Xn)
        auto first_xk = (*this)[*(begin + k)];
        set.insert(first_xk.begin(), first_xk.end());
    }

    return set;
}

void First::print() {
    for (auto& [nterm, terms] : first) {
        printf("First(%s%s%s) = { ", CG, to_string(nterm).data(), CD);
        for (auto& t : terms)
            printf("%s%s%s ", CY, to_string(t).data(), CD);
        printf("}\n");
    }
}

Follow::Follow(Grammar& g, Empty& empty, First& first) {
    // Follow(S) := { $ }
    follow[NTerm::StatementList].insert(Terminal{ .is_kw = false, .tok = Token_EOF });

    bool changed;
    do {
        changed = false;
        for (auto& [nterm, rules] : g.rules) {
            // For all rules of type A -> xByCz..
            for (auto& rule : rules) {
                const auto& A = rule.lhs;
                // For all non-terminals to get A -> xBy
                for (auto B = rule.rhs.begin(); B != rule.rhs.end(); B++) {
                    if (B->is_term)
                        continue;
                    size_t init_size = follow[B->nterm].size();
                    // If y != eps
                    if (B + 1 != rule.rhs.end()) {
                        // Add all symbols from First(y) to Follow(B)
                        auto first_y = first.get(B + 1, rule.rhs.end());
                        follow[B->nterm].insert(first_y.begin(), first_y.end());
                    }
                    // If Empty(y) = true. NOTE: Assuming that Empty(eps) = true
                    if (empty.get(B + 1, rule.rhs.end())){
                        // Add all symbols from Follow(A) to Follow(B)
                        follow[B->nterm].insert(follow[A].begin(), follow[A].end());
                    }

                    if (follow[B->nterm].size() != init_size)
                        changed = true;
                }
            }
        }
    } while (changed);
}

void Follow::print() {
    for (auto& [nterm, terms] : follow) {
        printf("Follow(%s%s%s) = { ", CG, to_string(nterm).data(), CD);
        for (auto& t : terms)
            printf("%s%s%s ", CY, to_string(t).data(), CD);
        printf("}\n");
    }
}

void generate_predict(Grammar& g, Empty& empty, First& first, Follow& follow) {
    for (auto& [nterm, rules] : g.rules) {
        // For all rules of type A -> x
        for (auto& rule : rules) {
            // Predict(x) = First(x);
            auto first_x = first.get(rule.rhs.begin(), rule.rhs.end());
            rule.predict.insert(first_x.begin(), first_x.end());
            // if Empty(x) = true
            if (empty.get(rule.rhs.begin(), rule.rhs.end())) {
                // Predict(x) U= Follow(x)
                rule.predict.insert(follow[rule.lhs].begin(), follow[rule.lhs].end());
            }
        }
    }
}

void print_predict(Grammar& g) {
    int nth_rule = 1;
    for (auto& [nterm, rules] : g.rules) {
        // For all rules of type A -> x
        for (auto& rule : rules) {
            printf("Predict(P_%s%i%s) = { ", CB, nth_rule++, CD);
            for (auto& term : rule.predict)
                std::cout << CY << to_string(term) << CD " ";
            printf("}\n");
        }
    }
}

const char* VLINE = "│";
const char* HLINE = "─";
const char* CROSS = "┼";

std::optional<int> get_rule(Grammar& g, NTerm nterm, Terminal term) {
    int n_rule = 1;
    for (auto& [n, rules] : g.rules) {
        if (n != nterm) {
            n_rule += rules.size();
            continue;
        }
        for (auto& rule : rules) {
            auto t = std::find(rule.predict.begin(), rule.predict.end(), term);
            if (t != rule.predict.end()) {
                return n_rule;
            }
            n_rule++;
        }
        break;
    }
    return {};
}
std::optional<int> get_rule(Grammar& g, int n_nterm, int n_term) {
    NTerm nterm = (NTerm)n_nterm;
    Terminal t;
    if (n_term >= KW_COUNT) {
        t.is_kw = false;
        t.tok = TokenType(n_term - KW_COUNT);
    } else {
        t.is_kw = true;
        t.kw = Keyword(n_term);
    }
    return get_rule(g, nterm, t);
}

void print_lltable(Grammar& g) {
    std::cout << "LL-Table" << std::endl;

    size_t vheader_width = 0;
    for (int i = 0; i < (int)NTerm::count; i++)
        if (strlen(NTERM_NAMES[i]) > vheader_width)
            vheader_width = strlen(NTERM_NAMES[i]);
    vheader_width += 2 + 1;     // 2 spaces and 1 border on the right.

    std::vector<int> cell_widths(1 + KW_COUNT + TOK_COUNT);
    cell_widths[0] = vheader_width;

    for (int i = 0; i < vheader_width - 1; i++)
        printf(" ");
    printf("%s", VLINE);

    // Print first header row of terminals.
    // Print keywords
    for (int kw = 0; kw < KW_COUNT; kw++) {
        int kw_len = (int)to_string((Keyword)kw).length();
        cell_widths[1 + kw] = std::max(kw_len, 2) + 2 + 1;
        if (kw_len == 1)
            printf("  " CY "%s" CD " %s", to_string((Keyword)kw).data(), VLINE);
        else
            printf(" " CY "%s" CD " %s", to_string((Keyword)kw).data(), VLINE);
    }
    // Print tokentypes
    for (int tok = 0; tok < TOK_COUNT; tok++) {
        int tok_len = (int)to_string((TokenType)tok).length();
        cell_widths[1 + KW_COUNT + tok] = std::max(tok_len, 2) + 2 + 1;
        if (tok_len == 1)
            printf("  " CY "%s" CD " %s", to_string((TokenType)tok).data(), VLINE);
        else
            printf(" " CY "%s" CD " %s", to_string((TokenType)tok).data(), VLINE);
    }
    printf("\n");

    const auto& separator = [&](){
        for (int i = 0; i < cell_widths.size(); i++) {
            for (int j = 0; j < cell_widths[i] - 1; j++) {
                printf("%s", HLINE);
            }
            printf("%s", CROSS);
        }
        printf("\n");
    };

    separator();

    for (int i = 0; i < (int)NTerm::count; i++) {
        NTerm nterm = (NTerm)i;
        std::cout << " " << CG << std::left << std::setw(vheader_width - 3) << to_string(nterm) << CD;
        std::cout << " " << VLINE;
        // Print keywords
        for (int kw = 0; kw < KW_COUNT; kw++) {
            int width_idx = 1 + kw;
            auto rule_n = get_rule(g, i, width_idx - 1);
            if (rule_n.has_value()) {
                std::cout << " " << CB << std::right << std::setw(cell_widths[width_idx] - 3)
                    << rule_n.value() << CD;
                std::cout << " " << VLINE;
            } else {
                std::cout << std::right << std::setw(cell_widths[width_idx] - 1) << " ";
                std::cout << VLINE;
            }
        }
        // Print tokentypes
        for (int tok = 0; tok < TOK_COUNT; tok++) {
            int width_idx = 1 + KW_COUNT + tok;
            auto rule_n = get_rule(g, i, width_idx - 1);
            if (rule_n.has_value()) {
                std::cout << " " << CB << std::right << std::setw(cell_widths[width_idx] - 3)
                    << rule_n.value() << CD;
                std::cout << " " << VLINE;
            } else {
                std::cout << std::right << std::setw(cell_widths[width_idx] - 1) << " ";
                std::cout << VLINE;
            }
        }

        std::cout << std::endl;
        separator();
    }
}

template<>
struct std::hash<std::pair<NTerm, Terminal>> {
    size_t operator()(const std::pair<NTerm, Terminal>& p) const {
        return TerminalIterator::term_to_val(p.second) * (int)NTerm::count + (int)p.first;
    }
};

bool check_predict(Grammar& g) {
    std::unordered_map<std::pair<NTerm, Terminal>, std::vector<int>> lltable;

    // Fill lltable.
    int n_rule = 1;
    for (auto& [nterm, rules] : g.rules) {
        for (auto& rule : rules) {
            for (auto term_it = TerminalIterator::begin(); term_it != TerminalIterator::end(); ++term_it) {
                Terminal term = *term_it;
                auto t = std::find(rule.predict.begin(), rule.predict.end(), term);
                if (t != rule.predict.end()) {
                    lltable[{ nterm, term }].push_back(n_rule);
                }
            }
            n_rule++;
        }
    }

    // Check if any cell has more that one rules to use.
    for (auto& [p, rules] : lltable) {
        auto& [nterm, term] = p;
        if (rules.size() > 1) {
            fprintf(stderr,
                    CR "error" CD ": check_predict(): "
                    "Combination [ " CG "%s" CD ", " CY "%s" CD " ] has more than single rule available ( ",
                    to_string(nterm).data(), to_string(term).data());
            for (auto& rule : rules) {
                fprintf(stderr, "P_" CM "%i" CD " ", rule);
            }
            fprintf(stderr, ").\n");
        }
    }

    return true;
}

