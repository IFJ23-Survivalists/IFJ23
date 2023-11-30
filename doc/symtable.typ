= Symbol Table <symtable>
The symbol table plays a crucial role in a compiler, and in this project, we have implemented it using an *AVL tree*. This self-balancing tree allows us to easily predict its performance. The implementation resides in `symtable.c`, with its methods exported in the header file `symtable.h`. Here are the core functions of the symbol table:

- *`symtable_init(symtable)`*: Initializes the symbol table

- *`symtable_free(symtable)`*: Releases all resources held by the given symbol table

- *`symtable_insert_function/variable(symtable, id, symbol)`*: Insert a function or variable symbol into the table. Return `false` if another value with the same identifier exists; otherwise, returns `true`. This may set error flag to `InternalErro` if it cannot allocate memory to insert into the tree.

- *`symtable_get_function/variable(symtable, id)`*: Retrieves the function or variable symbol of the given identifier.

- *`symtable_get_symbol_type(symtable, key)`*: Returns the type of the symbol if it exists. This function can also be used to check for the existence of any data in the table with the given key.

Note that the symbol table does not implement a `delete` operation because it is not needed for this project. Further details are available in the @semantic_analysis.
