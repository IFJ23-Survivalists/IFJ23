= Semantical Analysis <semantic_analysis>
Semantic analysis is a significant part of our program. We have integrated semantic checks directly into the syntactic analysis using the symbol table.

#v(1.0em)

#include "symtable.typ"

#v(1.0em)

== Function call parsing
In order to facilitate the handling of nested function calls within our compiler, it is imperative to maintain a structured hierarchy among function parameters. To address this requirement, we employ a stack structure implemented as a singly linked list. This stack serves as a repository for relevant information concerning each function and its parsed arguments.

The introduction of a new node to the stack occurs when the rule `E -> E,E` is applied, signifying the presence of scanned arguments for a new function. Subsequently, upon applying any of the function reduction rules, namely `E -> id(L)`, `E -> id(E)`, or `E -> id()`, the accumulated arguments are compared to the expected parameters. Following successful semantic checks and completion of the function call, the topmost node in the stack is then removed and all associated resources are freed.

#v(1.0em)

== Type checking
Semantic type checks attempt to convert two data types to the same data type whenever possible. Implicit conversion is supported exclusively for constants in precedence analysis, and it is crucial to maintain information about whether a given non-terminal originated from a constant. This information is propagated even through binary and unary expressions where each operand is a constant. Implicit conversion is permitted only for Int, Double data types, and nil values. nil is convertible to every nullable type. Consequently, expressions like Int? == nil are valid, as nil is converted into a nullable Int.
