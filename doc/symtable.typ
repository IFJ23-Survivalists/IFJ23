= Symbol Table <symtable>
Symbol table plays a crucial part of a compiler, in this project we have implemented it using AVL tree, since it's a self balancing tree that allows us easily predict its performance. The implementation is in `symtable.c` and its methods are exported in the header file `symtable.h`.

The symbol table plays a crucial role in a compiler, and in this project, we've implemented it using an AVL tree. This self-balancing tree enables us to predict its performance efficiently. The implementation resides in `symtable.c`, with its methods exported in the header file `symtable.h`. Here are the core functions of the symbol table:
- *`symtable_init(symtable)`*: Initialize the symbol table

- *`symtable_free(symtable)`*: Release all resources held by the given symbol table

- *`symtable_insert_function/variable(symtable, key, symbol)`*: Insert a function or variable symbol into the table. Return `false` if another value with the same key exists; otherwise, returns `true`.

- *`symtable_get_function/variable(symtable, key, symbol)`*: Retrieves the function or variable symbol.

- *`symtable_get_symbol_type(symtable, key)`*: Returns the type of the symbol if it exists. This function can also be used to check for the existence of any data in the table with the given key.

Note that the symbol table does not implement a `delete` operation in this project because it is not needed. Further details are available in the @semantic_analysis.
