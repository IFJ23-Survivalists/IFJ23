/**
 * @file llgen.h
 * @brief LL-Table generation structures and functions.
 */
#pragma once
#include "grammar.h"
#include <unordered_map>
#include <set>
#include <unordered_set>

/// Represents a function Empty, defined for all terminals and non-terminals.
struct Empty {
    std::unordered_map<NTerm, bool> empty;

    Empty(Grammar& g);

    inline bool operator[](NTerm n) { return empty[n]; };
    inline bool operator[](Terminal) { return false; };
    inline bool operator[](Symbol s) { return s.is_term ? false : empty[s.nterm]; }

    /**
     * @brief Get empty for range X1X2..Xn
     *
     * @param begin Iterator for X1
     * @param end Iterator after Xn
     * @return True if Empty(X1X2..Xn) = { eps }
     */
    bool get(const std::vector<Symbol>::iterator& begin,
             const std::vector<Symbol>::iterator& end);

    /// Print empty values for all non-terminals.
    void print();
};

/// Represents a function First, defined for all terminals and non-terminals.
struct First {
    std::unordered_map<NTerm, std::unordered_set<Terminal>> first;
    Empty& empty;

    First(Grammar& g, Empty& e);

    std::unordered_set<Terminal> operator[](const Symbol& s) {
        if (s.is_term)
            return { s.term };
        return first[s.nterm];
    }
    std::unordered_set<Terminal> get(const std::vector<Symbol>::iterator& begin,
                                     const std::vector<Symbol>::iterator& end);

    void print();
};

/// Represents a function Follow, defined for all non-terminals.
struct Follow {
    std::unordered_map<NTerm, std::unordered_set<Terminal>> follow;

    Follow(Grammar& g, Empty& empty, First& first);

    std::unordered_set<Terminal>& operator[](NTerm nterm) { return follow[nterm]; }

    void print();
};

/// Fill in the Grammar::rules::predict arrays using Empty First and Follow sets.
void generate_predict(Grammar& g, Empty& empty, First& first, Follow& follow);
/// Check if there is only one rule to use for each NTerm and Terminal combination and print error if needed.
bool check_predict(Grammar& g);

/// Print values of all predict sets for each NTerm.
void print_predict(Grammar& g);

/// Print LL-table for defined predict sets.
void print_lltable(Grammar& g);
