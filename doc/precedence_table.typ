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
        [],     [+-],[\*/],[logic],[??],[prefix],[postfix],[\(],[\)],[id],[,],[:],[\$],
        [+-],   [>],[<],[>],[>],[<],[<],[<],[>],[<],[>],[>],[>],
        [\*/],  [>],[>],[>],[>],[<],[<],[<],[>],[<],[>],[>],[>],
        [logic],[<],[<],[ ],[>],[<],[<],[<],[>],[<],[>],[>],[>],
        [??],   [<],[<],[<],[<],[<],[<],[<],[>],[<],[>],[ ],[>],
        [prefix],[>],[>],[>],[>],[ ],[<],[<],[>],[<],[>],[>],[>],
        [postfix],[>],[>],[>],[>],[>],[ ],[ ],[>],[<],[>],[>],[>],
        [(],    [<],[<],[<],[<],[<],[<],[<],[=],[<],[<],[ ],[ ],
        [)],    [>],[>],[>],[>],[>],[>],[ ],[>],[ ],[>],[>],[>],
        [id],   [>],[>],[>],[>],[>],[>],[=],[>],[ ],[>],[=],[>],
        [,],    [<],[<],[<],[<],[<],[<],[<],[>],[<],[>],[ ],[ ],
        [:],    [<],[<],[<],[<],[<],[<],[<],[>],[<],[>],[ ],[ ],
        [\$],   [<],[<],[<],[<],[<],[<],[<],[ ],[<],[ ],[ ],[ ],
    )
]

#underline[*Legend*]
#table(
    inset: (left:2.4em),
    stroke: none,
    columns: 2,
    align: left,
    [/ _logic_:],  [!= <= >= < > && ||],
    [/ _prefix_:], [negation arithmetic (-) or logical (!)],
    [/ _postfix_:],[unwrapping (!)],
    [/ _id_:],     [variable or constant],
    [/ _\$_:],     [every token that is no part of any precedence category e.g. '`->`' or '`{`']
)
_Note: The legend only describes categories in which it is unclear which tokens belong to them._

#v(1.0em)

Whenever a '`>`' relationship exists between the topmost pushdown token and the scanned token, the pushdown is reduced based on the definition of one of the @prules[ruduction rule]. In the case of an unknown rule at the top of the pushdown, such as `$<+E`, the precedence analysis will determine it to be a syntax error. This is because there is no @prules[ruduction rule] that corresponds to the expression `+E`.


== Reduction rules <prules>
The rules below represent the correct syntax for expressions. Each rule serves as a representative for all others with the same precedence and syntax prescription. For instance, the rule `E -> -E` is indicative of reducing unary prefix operators in expressions such as `-E` or `!E`.

#table(
    inset: (left:2.4em),
    stroke: none,
    columns: 2,
    align: left,
    [ *Rules*],       [*Handle*],
    [- E `->` id],    [_identifier or constant_],
    [- E `->` (E)],   [_removing parentheses_],
    [- E `->` -E],    [_negations_],
    [- E `->` E!],    [_unwrapping_],
    [- E `->` E + E], [_arithmetic operations_],
    [- E `->` E \* E],[_arithmetic operations_],
    [- E `->` E > E], [_logical operations_],
    [- E `->` E ? E], [_?? nil coalescing_],
    [- L `->` E , E], [_2 function arguments_],
    [- L `->` L , E], [_more then 2 function arguments_],
    [- E `->` id(L)], [_function call with multiple arguments_],
    [- E `->` id(E)], [_function call with a single argument_],
    [- E `->` id()],  [_function call without arguments_],
    [- E `->` id: E], [_named argument_],
)

Operator precedence analysis concludes when the currently scanned token lacks a precedence relationship with the topmost token on the pushdown. In such cases, the pushdown is recursively reduced until an applicable rule is found. If no rule is applicable, and the pushdown is reduced to only one non-terminal, the expression is deemed syntactically valid. The recursive parser then continues parsing. This processing approach may lead to premature and successful parsing completion even for syntactically invalid expressions. For instance, in the case of `1 == 1 == 1`, the parser successfully reduces `1 == 1` to a boolean expression `E`. However, it ends when encountering the operator `==`, which has no precedence relationship with the preceding `==` operator. While the bottom-up parsing succeeds, the recursive parser identifies the == operator without a corresponding rule, leading to a syntax error.
