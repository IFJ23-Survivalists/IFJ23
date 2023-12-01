/**
 * @brief Definitions for `symstack.h`.
 * @file symstack.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 25/11/2023
 */
#include "symstack.h"

SymStack* g_symstack = NULL;

// Check if `ss` argument is NULL or not. This can be made to do nothing in release build.
// TODO: Check only in release build.
#define CHECK_SS(ret) do {   \
        if (g_symstack == NULL) {          \
            SET_INT_ERROR(IntError_Runtime, "SymStack is not initialized. Call symstack_init() first."); \
            return ret;   \
        }                   \
    } while (false)

bool symstack_init() {
    g_symstack = malloc(sizeof(SymStack));
    if (!g_symstack) {
        SET_INT_ERROR(IntError_Memory, "symstack_init: Malloc failed");
        return false;
    }
    g_symstack->top = NULL;
    g_symstack->size = 0;
    return true;
}

void symstack_free() {
    CHECK_SS();
    while (g_symstack->top) {
        SymStackNode* aux = g_symstack->top;
        g_symstack->top = g_symstack->top->next;
        symtable_free(&aux->symtable);
        free(aux);
    }
    g_symstack->size = 0;
    free(g_symstack);
    g_symstack = NULL;
}

void symstack_clear() {
    CHECK_SS();
    while (g_symstack->top) {
        SymStackNode* aux = g_symstack->top;
        g_symstack->top = g_symstack->top->next;
        symtable_free(&aux->symtable);
        free(aux);
    }
    g_symstack->size = 0;
    g_symstack->top = NULL;
}

bool symstack_empty() {
    CHECK_SS(false);
    return g_symstack->size == 0;
}

size_t symstack_size() {
    CHECK_SS(0);
    return g_symstack->size;
}

Symtable* symstack_top() {
    CHECK_SS(NULL);
    return &g_symstack->top->symtable;
}

Symtable* symstack_bottom() {
    CHECK_SS(NULL);
    SymStackNode* node;
    for (node = g_symstack->top; node->next; node = node->next)
        ;
    return &node->symtable;
}

Symtable* symstack_push() {
    CHECK_SS(NULL);
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

    node->next = g_symstack->top;
    g_symstack->top = node;
    g_symstack->size++;

    return &g_symstack->top->symtable;
}

bool symstack_pop() {
    CHECK_SS(false);
    if (g_symstack->size == 0) {
        SET_INT_ERROR(IntError_Runtime, "Cannot call symstack_pop on empty symstack.");
        return false;
    }

    // Set second node as first.
    SymStackNode* aux = g_symstack->top;
    g_symstack->top = g_symstack->top->next;
    g_symstack->size--;

    // Delete first node and free symtable associated with it.
    symtable_free(&aux->symtable);
    if (got_error()) {
        free(aux);
        return false;
    }
    free(aux);

    return true;
}

Symtable* symstack_search(const char* sym_name) {
    CHECK_SS(NULL);
    for (SymStackNode* node = g_symstack->top; node; node = node->next) {
        if (symtable_get_symbol_type(&node->symtable, sym_name) != NULL)
            return &node->symtable;
    }
    return NULL;
}

VariableSymbol* symstack_search_variable(const char* var_name) {
    CHECK_SS(NULL);
    for (SymStackNode* node = g_symstack->top; node; node = node->next) {
        VariableSymbol* sym;
        if ((sym = symtable_get_variable(&node->symtable, var_name)) != NULL)
            return sym;
    }
    return NULL;
}

