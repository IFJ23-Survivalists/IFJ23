/**
 * @brief Definitions for `symstack.h`.
 * @file symstack.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 25/11/2023
 */
#include "symstack.h"

// Check if `ss` argument is NULL or not. This can be made to do nothing in release build.
// TODO: Check only in release build.
#define CHECK_SS(ss, ret) do {   \
        if (!ss) {          \
            SET_INT_ERROR(IntError_InvalidArgument, "ss cannot be NULL"); \
            return ret;   \
        }                   \
    } while (false)

bool symstack_init(SymStack* ss) {
    CHECK_SS(ss, false);
    ss->top = NULL;
    ss->size = 0;
    return true;
}

void symstack_free(SymStack* ss) {
    CHECK_SS(ss, );
    while (ss->top) {
        SymStackNode* aux = ss->top;
        ss->top = ss->top->next;
        symtable_free(&aux->symtable);
        free(aux);
    }
    ss->size = 0;
}

void symstack_clear(SymStack* ss) {
    symstack_free(ss);
}

bool symstack_empty(const SymStack* ss) {
    MASSERT(ss != NULL, "ss cannot be NULL");
    return ss->size == 0;
}

Symtable* symstack_top(const SymStack* ss) {
    CHECK_SS(ss, NULL);
    return &ss->top->symtable;
}

Symtable* symstack_push(SymStack* ss) {
    CHECK_SS(ss, NULL);
    SymStackNode* node = malloc(sizeof(SymStackNode));
    if (!node) {
        SET_INT_ERROR(IntError_Memory, "Malloc for SymStackNode failed.");
        return NULL;
    }

    symtable_init(&node->symtable);
    if (got_error()) {
        free(node);
        return false;
    }

    node->next = ss->top;
    ss->top = node;
    ss->size++;

    return &ss->top->symtable;
}

bool symstack_pop(SymStack* ss) {
    CHECK_SS(ss, false);
    if (ss->size == 0) {
        SET_INT_ERROR(IntError_Runtime, "Cannot call symstack_pop on empty symstack.");
        return false;
    }

    // Set second node as first.
    SymStackNode* aux = ss->top;
    ss->top = ss->top->next;
    ss->size--;

    // Delete first node and free symtable associated with it.
    symtable_free(&aux->symtable);
    if (got_error()) {
        free(aux);
        return false;
    }
    free(aux);

    return true;
}

Symtable* symstack_search(const SymStack* ss, const char* sym_name) {
    CHECK_SS(ss, NULL);
    for (SymStackNode* node = ss->top; node; node = node->next) {
        if (symtable_get_symbol_type(&node->symtable, sym_name) != NULL)
            return &node->symtable;
    }
    return NULL;
}
