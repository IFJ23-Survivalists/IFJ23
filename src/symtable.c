/**
 * @file symtable.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @author Jakub Kloub, xkloub03, VUT FIT
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
        par->type = (DataType)0;
        par->is_named = false;
        string_init(&par->oname);
        string_init(&par->iname);
        string_init(&par->code_name);
    }
}

void function_parameter_free(FunctionParameter *par) {
    if (par) {
        string_free(&par->iname);
        string_free(&par->oname);
        string_free(&par->code_name);
    }
}

void function_symbol_init(FunctionSymbol *sym) {
    sym->params = NULL;
    sym->param_count = 0;
    sym->return_value_type = (DataType)0;
    code_buf_init(&sym->code);
}

void function_symbol_free(FunctionSymbol *sym) {
    for (int i = 0; i < sym->param_count; i++)
        function_parameter_free(sym->params + i);
    if (sym->params)
        free(sym->params);
    sym->params = NULL;
    sym->param_count = 0;
    code_buf_free(&sym->code);
}

int string_comp(const char* a, const char* b) {
    if (a == NULL || b == NULL)
        return -1;
    return strcmp(a, b);
}

int funciton_symbol_has_param(FunctionSymbol *sym, const char* oname, const char* iname) {
    for (int i = 0; i < sym->param_count; i++) {
        if (string_comp(sym->params[i].oname.data, oname) == 0)
            return 1;
        if (string_comp(sym->params[i].iname.data, iname) == 0)
            return 2;
    }
    return 0;
}

FunctionParameter* function_symbol_get_param_named(FunctionSymbol *sym, const char* oname) {
    for (int i = 0; i < sym->param_count; i++) {
        if (strcmp(sym->params[i].oname.data, oname) == 0)
            return sym->params + i;
    }
    return NULL;
}

bool function_symbol_insert_param(FunctionSymbol *sym, FunctionParameter param) {
    MASSERT(sym, "Function symbol cannot be NULL.");
    if (param.iname.length == 0) {
        SET_INT_ERROR(IntError_InvalidArgument, "function_symbol_insert_param: iname cannot be of length 0.");
        return false;
    }

    // Resize the parameter array to fit the new parameter.
    sym->params = realloc(sym->params, (sym->param_count + 1) * sizeof(FunctionParameter));
    if (!sym->params) {
        SET_INT_ERROR(IntError_Memory, "function_symbol_insert_param: Realloc failed.");
        return false;
    }

    // Insert the new parameter at the end.
    sym->params[sym->param_count++] = param;
    return true;
}

bool function_symbol_emplace_param(FunctionSymbol *sym, DataType type, const char* oname, const char* iname) {
    MASSERT(sym && iname, "Function symbol and iname cannot be NULL.");

    // Create new FunctionParameter
    FunctionParameter p;
    function_parameter_init(&p);
    p.type = type;
    string_concat_c_str(&p.iname, iname);
    if (oname)
        string_concat_c_str(&p.oname, oname);
    p.is_named = oname != NULL;

    return function_symbol_insert_param(sym, p);
}

void variable_symbol_init(VariableSymbol *var) {
    var->type = (DataType)0;
    var->is_initialized = false;
    var->allow_modification = false;
    string_init(&var->code_name);
}

void variable_symbol_free(VariableSymbol *var) {
    string_free(&var->code_name);
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
        function_symbol_free(&aux->value.function);
    else
        variable_symbol_free(&aux->value.variable);

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
