/**
 * @note Project: Implementace překladače imperativního jazyka IFJ23
 * @brief Definition of ADT dynamic pushdownay.
 * @author Jakub Kloub, xkloub03, FIT VUT
 * @author Matúš Moravčík, xmorav48, FIT VUT
 * @date 23/11/2023
 */
#include "pushdown.h"
#include <stdlib.h>
#include "error.h"

void pushdown_init(Pushdown* pushdown) {
    pushdown->first = NULL;
    pushdown->last = NULL;
}

void pushdown_free(Pushdown* pushdown) {
    while (pushdown->first != NULL) {
        PushdownItem* to_delete = pushdown->first;
        pushdown->first = to_delete->next;
        if (to_delete->nterm != NULL) {
            if (to_delete->nterm->code_name != NULL)
                free(to_delete->nterm->code_name);
            free(to_delete->nterm);
        }
        free(to_delete);
    }
}

void pushdown_insert_last(Pushdown* pushdown, PushdownItem* value) {
    if (value == NULL)
        SET_INT_ERROR(IntError_InvalidArgument, "Null cannot be inserted into pushdown");

    if (pushdown->first == NULL)
        pushdown->first = value;

    if (pushdown->last != NULL) {
        pushdown->last->next = value;
        value->prev = pushdown->last;
    }

    pushdown->last = value;
}

void pushdown_insert_first(Pushdown* pushdown, PushdownItem* value) {
    if (value == NULL)
        SET_INT_ERROR(IntError_InvalidArgument, "Null cannot be inserted into pushdown");

    if (pushdown->last == NULL)
        pushdown->last = value;

    if (pushdown->first != NULL) {
        pushdown->first->prev = value;
        value->next = pushdown->first;
    }
    pushdown->first = value;
}

void pushdown_insert_after(Pushdown* pushdown, PushdownItem* item, PushdownItem* value) {
    if (value == NULL)
        SET_INT_ERROR(IntError_InvalidArgument, "Null cannot be inserted into pushdown");

    if (item == NULL) {
        pushdown_insert_first(pushdown, value);
        return;
    }

    if (item == pushdown_last(pushdown)) {
        pushdown->last = value;
    }

    PushdownItem* next_item = item->next;
    value->next = next_item;
    value->prev = item;
    item->next = value;

    if (next_item != NULL)
        next_item->prev = value;
}

PushdownItem* pushdown_last(const Pushdown* pushdown) {
    return pushdown->last;
}

PushdownItem* pushdown_next(const PushdownItem* item) {
    return item->next;
}

PushdownItem* pushdown_search_name(Pushdown* pushdown, char name) {
    PushdownItem* item = pushdown_last(pushdown);
    while (item != NULL && item->name != name)
        item = item->prev;

    return item;
}

PushdownItem* pushdown_search_terminal(struct Pushdown* pushdown) {
    PushdownItem* item = pushdown_last(pushdown);
    while (item != NULL && item->term == NULL)
        item = item->prev;

    return item;
}

void pushdown_remove_all_from_current(Pushdown* pushdown, PushdownItem* item) {
    if (item == NULL)
        SET_INT_ERROR(IntError_InvalidArgument, "`item` cannot be NULL");

    PushdownItem* to_delete = item;

    if (item->prev == NULL) {
        pushdown->first = NULL;
        pushdown->last = NULL;
    } else {
        item->prev->next = NULL;
        pushdown->last = item->prev;
    }

    while (to_delete != NULL) {
        PushdownItem* next = to_delete->next;
        free(to_delete);
        to_delete = next;
    }
}

PushdownItem* create_pushdown_item(Token* term, struct NTerm* nterm) {
    PushdownItem* item = malloc(sizeof(PushdownItem));
    item->nterm = nterm;
    item->term = term;
    item->name = '|';  // default name: end of rule
    item->next = NULL;
    item->prev = NULL;
    return item;
}
