= Semantical Analysis <semantic_analysis>
Semantic analysis is a significant part of our program. We have integrated semantic checks directly into the syntactic analysis using the symbol table.

#include "symtable.typ"

== Function call parsing
In order to facilitate the handling of nested function calls within our compiler, it is imperative to maintain a structured hierarchy among function parameters. To address this requirement, we employ a stack structure implemented as a singly linked list. This stack serves as a repository for relevant information concerning each function and its parsed arguments.

The introduction of a new node to the stack occurs when the rule `E -> E , E` is applied, signifying the presence of scanned arguments for a new function. Subsequently, upon applying any of the function reduction rules, namely `E -> id(L)`, `E -> id(E)`, or `E -> id()`, the accumulated arguments are compared to the expected parameters. Following successful semantic checks and completion of the function call, the topmost node in the stack is then removed and all associated resources are freed.

