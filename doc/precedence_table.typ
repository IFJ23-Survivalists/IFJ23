== Operator precedence table
The table illustrates the relationship between the precedence category of the topmost token (rows) from the pushdown and the precedence category of the currently scanned token (columns). The values in the table indicate which token possesses a higher precedence. Tokens are organized into precedence categories that have the same precedence level compared to other precedence categories (see legend).

#figure(
    kind: table,
    caption: [Operator precedence table]
)[
    #table(
        columns: 13,
        fill: (x, y) =>
            if x > 0 and y == 0 { yellow }
            else if x == 0 and y > 0 { aqua }
            else { white }
        ,
        align: center
        ,
        [],     [+-],[\*/],[logical],[??],[prefix],[postfix],[\(],[\)],[id],[,],[:],[\$],
        [+-],   [<],[>],[<],[<],[>],[>],[>],[<],[>],[<],[<],[<],
        [\*/],  [<],[<],[<],[<],[>],[>],[>],[<],[>],[<],[<],[<],
        [logical],[>],[>],[ ],[<],[>],[>],[>],[<],[>],[<],[<],[<],
        [??],   [>],[>],[>],[>],[>],[>],[>],[<],[>],[<],[ ],[<],
        [prefix],[<],[<],[<],[<],[ ],[>],[>],[<],[>],[<],[<],[<],
        [postfix],[<],[<],[<],[<],[<],[ ],[ ],[<],[>],[<],[<],[<],
        [(],    [>],[>],[>],[>],[>],[>],[>],[=],[>],[>],[ ],[ ],
        [)],    [<],[<],[<],[<],[<],[<],[ ],[<],[ ],[<],[=],[<],
        [id],   [<],[<],[<],[<],[<],[<],[=],[<],[ ],[<],[=],[<],
        [,],    [>],[>],[>],[>],[>],[>],[>],[<],[>],[<],[ ],[ ],
        [:],    [>],[>],[>],[>],[>],[>],[>],[<],[>],[<],[ ],[ ],
        [\$],   [>],[>],[>],[>],[>],[>],[>],[ ],[>],[ ],[ ],[ ],
    )
]

*Legend*
    - _logical_: == != <=' \>\= \< \> \&\& \|\|
    - _prefix_: \- \!
    - _postfix_: \!
    - _id_: variable or constant
    - _\$_: every token that is no part of any precedence category e.g. `->`

Whenever a `>` relationship exists between the topmost pushdown token and the scanned token, the pushdown is reduced based on the definition of one of the @prules. In the case of an unknown rule at the top of the pushdown, such as `$<+E`, the precedence analysis will determine it to be a syntax error. This is because there is no rule in @prules that corresponds to the expression `+E`.


== Precedence rules <prules>
The rules below represent the correct syntax for expressions. Each rule serves as a representative for all others with the same precedence and syntax prescription. For instance, the rule `E -> -E` is indicative of reducing unary prefix operators in expressions such as `-E` or `!E`.
- E -> id
- E -> (E)
- E -> -E
- E -> E!
- E -> E + E
- E -> E \* E
- E -> E > E
- E -> E ? E
- E -> E , E
- E -> L , E
- E -> id(L)
- E -> id(E)
- E -> id()
- E -> id: E

Operator precedence analysis concludes when the currently scanned token lacks a precedence relationship with the topmost token on the pushdown. In such cases, the pushdown is recursively reduced until an applicable rule is found. If no rule is applicable, and the pushdown is reduced to only one non-terminal, the expression is deemed syntactically valid. The recursive parser then continues parsing. This processing approach may lead to premature and successful parsing completion even for syntactically invalid expressions. For instance, in the case of `1 == 1 == 1`, the parser successfully reduces `1 == 1` to a boolean expression E. However, it ends when encountering the operator `==`, which has no precedence relationship with the preceding `==` operator. While the bottom-up parsing succeeds, the recursive parser identifies the == operator without a corresponding rule, leading to a syntax error.
