/**
 * @file symtable.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @author Jakub Kloub, xkloub03, VUT FIT
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
#include "codegen.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @struct FunctionParameter
 * @brief Represennts a single function parameter
 */
typedef struct {
    DataType type;          ///< Datatype of parameter
    /**
     * @brief Identifies if the parameter has outside name or not.
     * @warning When true, then the value of ::FunctionParameter::out_name is undefined
     */
    bool is_named;
    String iname;        ///< Name of the parameter when calling the function
    String oname;        ///< Name of the parameter when inside the function
    String code_name;    ///< Name of the parameter when it is inserted into temporary frame during function calls.
} FunctionParameter;

/**
 * @struct FunctionSymbol
 * @brief Represents a function symbol in the symbol table.
 */
typedef struct {
    int param_count;               /**< Number of items in ::FunctionSymbol::parameters. */
    FunctionParameter* params;     /**< Parameters of the function. */
    DataType return_value_type;    /**< Value the function returns or ::DataType_Undefined when it doesn't return anything. */
    CodeBuf code;      /**< < Generated code for this function. */
    CodeBuf code_defs;  /**< All variable definitions in given function local scope will be in this buffer. **/
    String code_name;   /**< IFJcode23 label */
    bool is_used;       /** Check if we should add this function to the resulting IFJcode23 */
} FunctionSymbol;

/**
 * @struct VariableSymbol
 * @brief Represents a variable symbol in the symbol table.
 */
typedef struct {
    DataType type;             /**< Data type of the variable. */
    bool is_initialized;       /**< Indicates if the variable is initialized. */
    bool allow_modification;   /**< Indicates if the variable can be modified. */
    /**
     * @brief Name of the variable in IFJcode23
     *
     * Name of the variable in IFJcode23 under which in can be fully referenced.
     * @note This name already includes information about which frame the variable is defined in.
     * @note This variable is to be created by `::parser_assign_code_name()` funciton call.
     * @note This variable if free'd during symtable destruction.
     */
    String code_name;
    /// Frame of the variable in IFJCode23
    Frame code_frame;
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
 * @brief Initialize the function symbol.
 * @param[in,out] sym Function symbol to initialize.
 * @warning Using FunctionSymbol without initializing could cause some memory to not be free'd during `symtable_free()`, because there could be uninitialized values.
 */
void function_symbol_init(FunctionSymbol *sym);

/**
 * @brief Free all memory resources used by ::FuncitonSymbol
 * @param[in,out] sym Function symbol to free resources of.
 */
void function_symbol_free(FunctionSymbol *sym);

/**
 * @brief Check if FunctionSymbol already contains given parameter.
 * @param[in] sym Function symbol to search in.
 * @param[in] oname Name of the parameter when calling the function.
 * @param[in] iname Name of the parameter inside the function.
 * @return 0 when the parameter in NOT present. Otherwise 1 if `oname` exist or 2 if iname `exists`.
 */
int funciton_symbol_has_param(FunctionSymbol *sym, const char* oname, const char* iname);

/**
 * @brief Get function parameter by outside name.
 * @param[in] sym FunctinoSymbol to look for the parameter in.
 * @param[in] oname Name of the parameter when calling the function.
 * @return Pointer to function parameter on NULL when not found.
 */
FunctionParameter* function_symbol_get_param_named(FunctionSymbol *sym, const char* oname);

/**
 * @brief Insert function parameter to parameters in function symbol.
 * @param[in,out] sym FunctionSymbol to insert the parameter into.
 * @param[in] param Initialized parameter to insert.
 * @note Parameter parm is NOT deeply copied and will take ownership of this parameter.
 * @return True if insertion was successful, False otherwise.
 */
bool function_symbol_insert_param(FunctionSymbol *sym, FunctionParameter param);

/**
 * @brief Construct FunctionParameter and insert it into parameter array.
 * @param[in,out] sym FunctionSymbol to insert the new FunctionParameter in.
 * @param[in] type ::DataType of parameter.
 * @param[in] oname Name of the parameter in function calls or NULL for unnamed parameter.
 * @param[in] iname Name of the parameter inside function definition.
 * @return True if insertion was successful, False otherwise.
 */
bool function_symbol_emplace_param(FunctionSymbol *sym, DataType type, const char* oname, const char* iname);

/**
 * @brief Initialize the variable symbol.
 * @param[in,out] var Variable symbol to initialize.
 * @note This will set all attributes to false and datatype to 0.
 */
void variable_symbol_init(VariableSymbol *var);

/**
 * @brief Free all resources used by VariableSymbol.
 * @param[in,out] var VariableSymbol to free resources of.
 */
void variable_symbol_free(VariableSymbol *var);

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
