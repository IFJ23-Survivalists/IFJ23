= Info
This section deals with the organization of our project.
== Work division
In the end, we have allocated the tasks as follows:
- Le Duy Nguyen (xnguye27) - 29%
    - Lexical analysis and everything it involves.
    - Core implementation of symbol table.
    - LL-grammar
    - Code generation API and implementation
    - Tests
    - Documentation
    - Makefile
- Jakub Kloub (xkloub03) - 28%
    - Syntactical and semantical analysis in recursive descent.
    - Code generation (using the API)
    - Symbol table extensions and fixes.
    - LL-grammar and LL-table.
    - Tests
    - Documentation
- Matúš Moravčík (xmorav48) - 28%
    - Syntactical and semantical analysis in precedence analysis.
    - Precedence table
    - Tests
    - Code generation (using the API)
    - Documentation
- Lukáš Habr (xhabrl01) - 15%
    - LL-grammar
    - Code generation API
    - Builtin functions.
    - Documentation

The reason behind _Lukáš Habr_ having less points, is that he contributed significantly less to the project due to falling ill, which limited his availability to work on our project for an extended period. Consequently, a substantial portion of his assigned tasks was redistributed among the remaining three team members.

== The team's working method
Our team collectively decided to utilize GitHub as our version control system. Additionally, we have held several meetings during which we determined key factors for our project. For each big part of the project, we created a development branch, which we finally merged to get the final product.

We have implemented tests for the majority of our project components. This way, whenever someone makes changes, they can verify whether it constitutes a breaking change for other functionalities. Additionally, these tests serve as the foundation for an automated GitHub action that notifies us of any compilation errors or test failures.

As a communication plaform, we mainly used Discord along with GitHub pull requests.

== Development cycle
All team members worked concurrently on their designated components. Initially, we focused on defining the API, and once it became feasible, we also developed tests for this interface. This approach led to a test-driven development for certain aspects of our project.

The chronological order of our project completion can be summarized as follows:
+ Implementation of a functional lexical analyzer + tests + bug fixes
+ Implementation of a functional syntactical analyzer + tests + bug fixes
+ Implementation of a functional semantical analyzer + tests + bug fixes
+ Implementation of functional code generation + tests + bug fixes
+ Tests and bug fixes

== Project file structure
Key files of our projects are:
#table(
    columns: 2,
    stroke: none,
    [*Filename*], [*Description*],
    [main.c], [_Main entry point to our program. We tried to keep it simple and short._],
    [scanner.c], [_Lexer implementation_],
    [parser.c], [_Main parser entry point. This file controls the flow of our two-phase analysis and final code generation._],
    [rec_parser.c], [_Implements the second phase of recursive descent._],
    [rec_parser_collect.c], [_Implements the first phase of recursive descent._],
    [expr_parser.c], [_Implements the precedence analysis._],
    [codegen.c], [_Wrapper for generating the final IFJcode23 instructions._],
    [builtin.c], [_Generates builtin functions into symbol table (which then contains their code and declaration information)_],
    [symtable.c], [_Implementation of balanced AVL tree representing our symbol table._],
    [string.c], [_String library used for managing string across our program._],
    [symstack.c], [_Stack of symtables used during parsing._],
    [error.c], [_Unify error output across our entire program. Also provides some additional useful macros such as `MASSERT`._],
)
