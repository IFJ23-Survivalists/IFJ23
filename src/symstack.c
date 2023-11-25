/**
 * @brief Definitions for `symstack.h`.
 * @file symstack.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 25/11/2023
 */
#include "symstack.h"

bool symstack_init(SymStack* ss) { (void)ss; return false; }
void symstack_free(SymStack* ss) { (void)ss; }
void symstack_clear(SymStack* ss) { (void)ss; }
bool symstack_empty(const SymStack* ss) { (void)ss; return false; }
Symtable* symstack_top(const SymStack* ss) { (void)ss; return NULL; }
Symtable* symstack_push(SymStack* ss) { (void)ss; return NULL; }
bool symstack_pop(SymStack* ss) { (void)ss; return false; }
Symtable* symstack_search(const SymStack* ss, const char* sym_name) { (void)ss; (void)sym_name; return NULL; }
