/**
 * @file test/scanner.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 10/10/2023
 * @brief Tester for scanner.h
 */

#include "../pushdown.h"
#include "../scanner.h"
#include "test.h"

int main() {
    atexit(summary);

    Pushdown pushdown;
    Token token;
    NTerm* nonterm = malloc(sizeof(NTerm));

    PushdownItem* rule_end_marker;
    PushdownItem* rule_end_marker2;
    PushdownItem* term;
    PushdownItem* nterm;

    suite("Test pushdown_init") {
        pushdown_init(&pushdown);
        test(pushdown.first == NULL);
        test(pushdown.last == NULL);
    }

    suite("Test create_pushdown_item") {
        rule_end_marker = create_pushdown_item(NULL, NULL);
        test(rule_end_marker->name == '|');
        test(rule_end_marker->term == NULL);
        test(rule_end_marker->nterm == NULL);
        term = create_pushdown_item(&token, NULL);
        test(term->term != NULL);
        test(term->nterm == NULL);
        nterm = create_pushdown_item(NULL, nonterm);
        test(nterm->term == NULL);
        test(nterm->nterm != NULL);
        rule_end_marker2 = create_pushdown_item(NULL, NULL);
    }

    suite("Test insert_last") {
        pushdown_insert_last(&pushdown, term);
        test(pushdown.first == term);
        test(pushdown.last == term);
        test(term->next == NULL);
        test(term->prev == NULL);
        pushdown_insert_last(&pushdown, nterm);
        test(pushdown.last == nterm);
        test(nterm->prev == term);
        test(nterm->next == NULL);
    }

    suite("Test insert_after") {
        pushdown_insert_after(&pushdown, term, rule_end_marker);
        test(pushdown.first == term);
        test(pushdown.last == nterm);
        pushdown_insert_after(&pushdown, nterm, rule_end_marker2);
        test(pushdown.first == term);
        test(pushdown.last == rule_end_marker2);
    }

    suite("Test remove_all_from_current") {
        pushdown_remove_all_from_current(&pushdown, rule_end_marker);
        test(pushdown.first == term);
        test(pushdown.last == term);
        rule_end_marker2 = create_pushdown_item(NULL, NULL);
        pushdown_insert_after(&pushdown, term, rule_end_marker2);
        pushdown_remove_all_from_current(&pushdown, term);
        test(pushdown.first == NULL);
        test(pushdown.last == NULL);
    }

    suite("Test last") {
        test(pushdown_last(&pushdown) == NULL);
        rule_end_marker = create_pushdown_item(NULL, NULL);
        pushdown_insert_last(&pushdown, rule_end_marker);
        test(pushdown_last(&pushdown) == rule_end_marker);
        term = create_pushdown_item(&token, NULL);
        pushdown_insert_last(&pushdown, term);
        test(pushdown_last(&pushdown) == term);
    }

    suite("Test next") {
        test(pushdown_next(rule_end_marker) == term);
        test(pushdown_next(term) == NULL);
    }

    suite("Test search_name") {
        term->name = '+';
        test(pushdown_search_name(&pushdown, '|') == rule_end_marker);
        test(pushdown_search_name(&pushdown, '+') == term);
        nterm = create_pushdown_item(NULL, nonterm);
        nterm->name = 'E';
        pushdown_insert_last(&pushdown, nterm);
        test(pushdown_search_name(&pushdown, 'E') == nterm);
    }

    suite("Test pushdown_search_terminal") {
        test(pushdown_search_terminal(&pushdown) == term);
    }

    pushdown_free(&pushdown);
}
