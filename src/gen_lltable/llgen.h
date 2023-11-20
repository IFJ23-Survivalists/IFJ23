#pragma once
#include "grammar.h"
#include <unordered_map>
#include <set>
#include <unordered_set>

struct Empty {
    std::unordered_map<NTerm, bool> empty;

    Empty(Grammar& g);

    inline bool operator[](NTerm n) { return empty[n]; };
    inline bool operator[](Symbol s) { return s.is_term ? false : empty[s.nterm]; }
    inline bool operator[](Terminal) { return false; };

    bool get(const std::vector<Symbol>::iterator& begin,
             const std::vector<Symbol>::iterator& end);

    void print();
};

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

struct Follow {
    std::unordered_map<NTerm, std::unordered_set<Terminal>> follow;

    Follow(Grammar& g, Empty& empty, First& first);

    std::unordered_set<Terminal>& operator[](NTerm nterm) { return follow[nterm]; }

    void print();
};

void generate_predict(Grammar& g, Empty& empty, First& first, Follow& follow);
bool check_predict(Grammar& g);

void print_predict(Grammar& g);
void print_lltable(Grammar& g);
