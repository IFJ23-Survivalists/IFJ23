/**
 * @file symtable.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/10/2023
 * @brief Header file for a symbol table module used for managing variables and functions.
 *
 * This module provides data structures and functions for creating, managing, and querying a symbol
 * table used in language parsing and symbol resolution.
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "string.h"
#include "scanner.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @struct FunctionParameter
 * @brief Represents the parameters of a function.
 */
typedef struct {
    int argc;        /**< Number of function arguments. */
    DataType *argv;  /**< Array of argument data types. */
} FunctionParameter;

/**
 * @struct FunctionSymbol
 * @brief Represents a function symbol in the symbol table.
 */

typedef struct {
    FunctionParameter parameters; /**< Parameters of the function. */
    DataType return_value_type;       /**< Return value data type. */
} FunctionSymbol;

/**
 * @struct VariableSymbol
 * @brief Represents a variable symbol in the symbol table.
 */
typedef struct {
    DataType type;             /**< Data type of the variable. */
    bool is_defined;           /**< Indicates if the variable is defined. */
    bool allow_modification;   /**< Indicates if the variable can be modified. */
} VariableSymbol;

/**
 * @enum NodeType
 * @brief Represents the type of an item in the symbol table (variable or function).
 */
typedef enum {
    NodeType_Variable, /**< Represents a variable. */
    NodeType_Function  /**< Represents a function. */
} NodeType;

/**
 * @union NodeValue
 * @brief Represents the value of an item in the symbol table (variable or function).
 */
typedef union {
    VariableSymbol variable; /**< Value for a variable item. */
    FunctionSymbol function; /**< Value for a function item. */
} NodeValue;

/**
 * @struct Node
 * @brief Represents a node of an AVL tree that contains an item (variable or function) in the symbol table.
 */
typedef struct item_t {
    String key;             /**< Key (name) of the item. */
    NodeType type;          /**< Type of the node (a function or a variable)*/
    NodeValue value;        /**< Value of the node */
    struct item_t *left;    /**< Pointer to the left item */
    struct item_t *right;   /**< Pointer to the right item */
    int height;            /**< Height of the current AVL node*/
} Node;

/**
 * @struct Symtable
 * @brief Represents the symbol table.
 */
typedef struct {
    Node *root;
} Symtable;

/**
 * @brief Initialize a FunctionParameter struct.
 *
 * This function initializes a FunctionParameter struct with default values.
 *
 * @param[out] par The FunctionParameter struct to initialize.
 */
void function_parameter_init(FunctionParameter *par);

/**
 * @brief Free memory associated with a FunctionParameter struct.
 *
 * This function frees any dynamically allocated memory used by a FunctionParameter struct.
 *
 * @param[in,out] par The FunctionParameter struct to free.
 */
void function_parameter_free(FunctionParameter *par);

/**
 * @brief Add a parameter with a specified data type to a FunctionParameter struct.
 *
 * This function adds a parameter with the specified data type to a FunctionParameter struct.
 *
 * @param[in,out] par The FunctionParameter struct to which the parameter is added.
 * @param[in] type The data type of the parameter.
 */
void function_parameter_push(FunctionParameter *par, DataType type);

/**
 * @brief Initialize the function symbol.
 * @param[in,out] sym Function symbol to initialize.
 * @warning Using FunctionSymbol without initializing could cause some memory to not be free'd during `symtable_free()`, because there could be uninitialized values.
 */
void function_symbol_init(FunctionSymbol *sym);

/**
 * @brief Initialize the variable symbol.
 * @param[in,out] var Variable symbol to initialize.
 * @note This will set all attributes to false and datatype to 0.
 */
void variable_symbol_init(VariableSymbol *var);

/**
 * @brief Initialize a symbol table.
 *
 * This function initializes a symbol table, setting it up for use.
 *
 * @param[out] symtable The Symtable struct to initialize.
 */
void symtable_init(Symtable *symtable);

/**
 * @brief Free memory associated with a symbol table.
 *
 * This function frees any dynamically allocated memory used by the symbol table.
 *
 * @param[in,out] symtable The Symtable struct to free.
 */
void symtable_free(Symtable *symtable);

/**
 * @brief Insert a function symbol into the symbol table.
 *
 * This function inserts a function symbol into the symbol table with the specified key.
 *
 * @param[in,out] symtable The Symtable struct to insert into.
 * @param[in] key The key (name) of the function.
 * @return Successfully inserted into symtable or not
 * @param[in] function The FunctionSymbol to insert.
 */
bool symtable_insert_function(Symtable *symtable, const char *key, FunctionSymbol function);

/**
 * @brief Insert a variable symbol into the symbol table.
 *
 * This function inserts a variable symbol into the symbol table with the specified key.
 *
 * @param[in,out] symtable The Symtable struct to insert into.
 * @param[in] key The key (name) of the variable.
 * @param[in] variable The VariableSymbol to insert.
 * @return Successfully inserted into symtable or not
 */
bool symtable_insert_variable(Symtable *symtable, const char *key, VariableSymbol variable);

/**
 * @brief Get a function symbol from the symbol table by name.
 *
 * This function retrieves a function symbol from the symbol table by name (key).
 *
 * @param[in] symtable The Symtable struct to search.
 * @param[in] str The name of the function to retrieve.
 * @return A pointer to the FunctionSymbol if found; otherwise, NULL.
 */
FunctionSymbol *symtable_get_function(Symtable *symtable, const char *str);

/**
 * @brief Get a variable symbol from the symbol table by name.
 *
 * This function retrieves a variable symbol from the symbol table by name (key).
 *
 * @param[in] symtable The Symtable struct to search.
 * @param[in] str The name of the variable to retrieve.
 * @return A pointer to the VariableSymbol if found; otherwise, NULL.
 */
VariableSymbol *symtable_get_variable(Symtable *symtable, const char *str);

/**
 * @brief Get the data type of a symbol in the symbol table by name.
 *
 * This function retrieves the data type of a symbol from the symbol table
 * by its name (key). It returns a pointer to the NodeType representing the data type.
 *
 * @param[in] symtable The Symtable struct to search.
 * @param[in] str The name of the symbol for which to retrieve the data type.
 * @return A pointer to the NodeType representing the data type of the symbol if found;
 *         otherwise, NULL.
 */
NodeType *symtable_get_symbol_type(Symtable *symtable, const char *str);

#endif
