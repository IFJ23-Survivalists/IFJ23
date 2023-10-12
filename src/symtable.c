/**
 * @file symtable.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/10/2023
 * @brief Implementation for symtable.h
 */

#include "symtable.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"

void function_parameter_init(FunctionParameter *par) {
    if (par) {
        par->argc = 0;
        par->argv = NULL;
    }
}

void function_parameter_free(FunctionParameter *par) {
    if (par && par->argc) {
        free(par->argv);
        par->argv = NULL;
        par->argc = 0;
    }
}

void function_parameter_push(FunctionParameter *par, DataType type) {
    if (!par) {
        return;
    }

    DataType *reallocated = realloc(par->argv, sizeof(DataType) + (par->argc + 1));

    if (!reallocated) {
        set_error(Error_Internal);
        eprint("function_parameter_push: Out Of Memory\n");
        return;
    }

    reallocated[par->argc++] = type;
    par->argv = reallocated;
}

void symtable_init(Symtable *symtable) {
    if (symtable) {
        symtable->root = NULL;
    }
}

void item_free_childs(Item *item) {
    if (item->left) {
        free(item->left);
        item->left = NULL;
    }

    if (item->right) {
        free(item->right);
        item->right = NULL;
    }
}

void symtable_free(Symtable *symtable) {
    if (!symtable || !symtable->root) return;
    item_free_childs(symtable->root);
    free(symtable->root);
}

static Item *create_item(const char *key, ItemType type, ItemValue value) {
    Item *item = malloc(sizeof(Item));

    if (!item) {
        set_error(Error_Internal);
        return NULL;
    }

    item->key = string_from_c_str(key);
    item->type = type;
    item->value = value;
    item->left = NULL;
    item->right = NULL;

    if (got_error()) {
        free(item);
        return NULL;
    }

    return item;
}

static Item* item_bvs_insert(Item *item, const char *key, ItemType type, ItemValue value, bool *inserted) {
    if (!item) {
        Item *new_item = create_item(key, type, value);

        if (!new_item) {
            return false;
        }

        *inserted = true;
        return new_item;
    }

    int cmp = strcmp(key, item->key.data);

    if (cmp > 0) {
        item->right = item_bvs_insert(item->right, key, type, value, inserted);
    } else if (cmp < 0) {
        item->left = item_bvs_insert(item->left, key, type, value, inserted);
    }

    return item;
}


static bool symtable_insert(Symtable *symtable, const char *key, ItemType type, ItemValue value) {
    if (!symtable || !key) {
        return false;
    }

    bool inserted = false;
    symtable->root = item_bvs_insert(symtable->root, key, type, value, &inserted);
    return inserted;
}

static Item *item_bvs_get(Item *item, const char *key) {
    if (!item) {
        return NULL;
    }

    int cmp = strcmp(key, item->key.data);

    return cmp > 0 ? item_bvs_get(item->right, key)
         : cmp < 0 ? item_bvs_get(item->left, key)
         : item;
}

bool symtable_insert_function(Symtable *symtable, const char *key, FunctionSymbol function) {
    ItemValue value;
    value.function = function;
    return symtable_insert(symtable, key, ItemType_Function, value);
}

bool symtable_insert_variable(Symtable *symtable, const char *key, VariableSymbol variable) {
    ItemValue value;
    value.variable = variable;
    return symtable_insert(symtable, key, ItemType_Variable, value);
}

FunctionSymbol *symtable_get_function(Symtable *symtable, const char *key) {
    Item *item = item_bvs_get(symtable->root, key);

    if (item && item->type == ItemType_Function) {
        return &item->value.function;
    }

    return NULL;
}

VariableSymbol *symtable_get_variable(Symtable *symtable, const char *key) {
    Item *item = item_bvs_get(symtable->root, key);

    if (item && item->type == ItemType_Function) {
        return &item->value.variable;
    }

    return NULL;
}
