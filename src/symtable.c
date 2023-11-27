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

    DataType *reallocated = realloc(par->argv, sizeof(DataType) * (par->argc + 1));

    if (!reallocated) {
        set_error(Error_Internal);
        eprint("function_parameter_push: Out Of Memory\n");
        return;
    }

    reallocated[par->argc++] = type;
    par->argv = reallocated;
}

void function_symbol_init(FunctionSymbol *sym) {
    sym->parameters.argv = NULL;
    sym->parameters.argc = 0;
    sym->return_value_type = (DataType)0;
}

void variable_symbol_init(VariableSymbol *var) {
    var->type = (DataType)0;
    var->is_defined = false;
    var->allow_modification = false;
}

void symtable_init(Symtable *symtable) {
    if (symtable) {
        symtable->root = NULL;
    }
}

void node_free(Node **node) {
    Node *aux = *node;
    if (!aux)
        return;

    node_free(&aux->left);
    node_free(&aux->right);

    if (aux->type == NodeType_Function)
        function_parameter_free(&aux->value.function.parameters);

    string_free(&aux->key);

    free(aux);
    *node = NULL;
}

void symtable_free(Symtable *symtable) {
    if (!symtable || !symtable->root)
        return;
    node_free(&symtable->root);
}

static Node *create_node(const char *key, NodeType type, NodeValue value) {
    Node *node = malloc(sizeof(Node));

    if (!node) {
        set_error(Error_Internal);
        return NULL;
    }

    node->key = string_from_c_str(key);
    node->type = type;
    node->value = value;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;

    if (got_error()) {
        free(node);
        return NULL;
    }

    return node;
}

int max(int a, int b) {
    return a > b ? a : b;
}

char node_height(Node* node) {
    return node == NULL ? 0 : node->height;
}

Node* node_rotate_right(Node* y) {
    Node *x = y->left;
    y->left = x->right;
    x->right = y;

    y->height = max(node_height(y->left), node_height(y->right))+1;
    x->height = max(node_height(x->left), node_height(x->right))+1;

    return x;
}

Node* node_rotate_left(Node* x) {
    Node *y = x->right;
    x->right = y->left;
    y->left = x;

    y->height = max(node_height(y->left), node_height(y->right))+1;
    x->height = max(node_height(x->left), node_height(x->right))+1;

    return y;
}

static Node* node_bvs_insert(Node *node, const char *key, NodeType type, NodeValue value, bool *inserted) {
    if (!node) {
        Node *new_node = create_node(key, type, value);

        if (!new_node) {
            return false;
        }

        *inserted = true;
        return new_node;
    }

    int cmp = strcmp(key, node->key.data);

    if (cmp > 0) {
        node->right = node_bvs_insert(node->right, key, type, value, inserted);
    } else if (cmp < 0) {
        node->left = node_bvs_insert(node->left, key, type, value, inserted);
    }

    // Balance
    int balance = node_height(node->left) - node_height(node->right);

    // Left Left Case
    if (balance > 1 && strcmp(key, node->left->key.data) < 0) {
        return node_rotate_right(node);
    }

    // Right Right Case
    if (balance < -1 && strcmp(key, node->right->key.data) > 0) {
        return node_rotate_left(node);
    }

    // Left Right Case
    if (balance > 1 && strcmp(key, node->left->key.data) > 0) {
        node->left = node_rotate_left(node->left);
        return node_rotate_right(node);
    }

    // Right Left Case
    if (balance < -1 && strcmp(key, node->right->key.data) < 0) {
        node->right = node_rotate_right(node->right);
        return node_rotate_left(node);
    }

    return node;
}


static bool symtable_insert(Symtable *symtable, const char *key, NodeType type, NodeValue value) {
    if (!symtable || !key) {
        return false;
    }

    bool inserted = false;
    symtable->root = node_bvs_insert(symtable->root, key, type, value, &inserted);
    return inserted;
}

static Node *node_bvs_get(Node *node, const char *key) {
    if (!node) {
        return NULL;
    }

    int cmp = strcmp(key, node->key.data);

    return cmp > 0 ? node_bvs_get(node->right, key)
         : cmp < 0 ? node_bvs_get(node->left, key)
         : node;
}

bool symtable_insert_function(Symtable *symtable, const char *key, FunctionSymbol function) {
    NodeValue value;
    value.function = function;
    return symtable_insert(symtable, key, NodeType_Function, value);
}

bool symtable_insert_variable(Symtable *symtable, const char *key, VariableSymbol variable) {
    NodeValue value;
    value.variable = variable;
    return symtable_insert(symtable, key, NodeType_Variable, value);
}

FunctionSymbol *symtable_get_function(Symtable *symtable, const char *key) {
    Node *node = node_bvs_get(symtable->root, key);

    if (node && node->type == NodeType_Function) {
        return &node->value.function;
    }

    return NULL;
}

VariableSymbol *symtable_get_variable(Symtable *symtable, const char *key) {
    Node *node = node_bvs_get(symtable->root, key);

    if (node && node->type == NodeType_Variable) {
        return &node->value.variable;
    }

    return NULL;
}

NodeType *symtable_get_symbol_type(Symtable *symtable, const char *key) {
    Node *node = node_bvs_get(symtable->root, key);
    return node == NULL ? NULL : &node->type;
}
