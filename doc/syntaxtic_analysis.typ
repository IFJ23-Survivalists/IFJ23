= Syntaxtic analysis <syntaxtic_analysis>
We have chosen to implement syntactic analysis using recursive descent and precedence analysis.

== LL-grammar
This is our list of rules:
#block(
  fill: luma(230),
  inset: 8pt,
  radius: 4pt,
)[
+ $"<stmtList>" -> "<stmt> <stmtSeparator> <stmtList>"$
+ $"<stmtList>" -> epsilon$
+ $"<stmtSeparator>" -> "EOL"$
+ $"<stmtSeparator>" -> "}"$
+ $"<stmtSeparator>" -> "$"$
+ $"<stmt>" -> "while ( <expr> ) { <stmtList> } <stmtList>"$
+ $"<stmt>" -> "func ID ( <params> ) <funcReturnType> { <stmtList> } <stmtList>"$
+ $"<stmt>" -> "return <returnExpr>"$
+ $"<stmt>" -> "if <ifStmt>"$
+ $"<stmt>" -> "let ID <assignType> = <expr>"$
+ $"<stmt>" -> "var ID <assignType> <assignExpr>"$
+ $"<stmt>" -> "ID = <expr>"$
+ $"<stmt>" -> "<expr>"$
+ $"<funcReturnType>" -> "-> DataType"$
+ $"<funcReturnType>" -> epsilon$
+ $"<ifStmt>" -> "<ifCondition> { <stmtList> } <else>"$
+ $"<params>" -> "ID ID : DataType <params_n>"$
+ $"<params>" -> epsilon$
+ $"<params_n>" -> ", <params>"$
+ $"<params_n>" -> epsilon$
+ $"<returnExpr>" -> "<expr>"$
+ $"<returnExpr>" -> epsilon$
+ $"<ifCondition>" -> "( <expr> )"$
+ $"<ifCondition>" -> "let ID"$
+ $"<else>" -> "else <elseIf>"$
+ $"<else>" -> "<stmtList>"$
+ $"<elseIf>" -> "{ <stmtList> } <stmtList>"$
+ $"<elseIf>" -> "if <ifStmt>"$
+ $"<assignType>" -> ": DataType"$
+ $"<assignType>" -> epsilon$
+ $"<assignExpr>" -> "= <expr>"$
+ $"<assignExpr>" -> epsilon$
]

The grammar contains several ambiguities that are resolved in the code:
- `<expr>` -When a rule containing this non-terminal is used, we switch to precedence analysis.
- `<stmtSeparator>` - Specifically, rule #4, when used, does not extract the `'}'` token from the input tape. The reason is that this rule exists solely to separate statements with `'}'` without the need for a newline character between them. However, we do not want to consume the `'}'` here so that we can process it in rules `6`, `7`, etc.
- Rule `12` vs `13`:
    - The decision here is based on whether ID is an identifier for a function or a variable. If it's a function, we switch to precedence analysis using rule `13`; otherwise, we assume it's an assignment and use rule `12`.
    - The drawback of this approach is that we cannot support expressions like: ```swift
        var a = 0
        a + 1
    ``` where this is a dead code, so we have decided to not support these.
- Token `EOL` - the scanner counts it as whitespace along with comments. The way we distinguish it is by having an attribute in whitespace tokens indicating whether it contains an end-of-line or not.

The reason we have left those ambiguities here is that, they are either not solvable with LL(1) grammar, or it's much easier to solve them manually in the code.

== LL-table
Using the rules above, we have the following LL-table:
#figure(
    kind: table,
    caption: [LL-table 1/2]
)[
    #table(
        columns: (auto,auto,auto,auto,auto,auto,auto,auto,auto,auto,auto),
        fill: (x, y) =>
            if x > 0 and y == 0 { yellow }
            else if x == 0 and y > 0 { aqua }
            else { white }
        ,
        align: (x, y) =>
            if x == 0 and y == 0 { center }
            else if x == 0 { left }
            else { center }
        ,
        [nterm \\ term],     [\$],[\\n],[{],[}],[(],[)],[:],[\-\>],[=],[,],
        [\<stmtList\>],      [2],[2],[],[2],[],[],[],[],[],[],
        [\<stmtSeparator\>], [5],[3],[4],[],[],[],[],[],[],[],
        [\<stmt\>],          [],[],[],[],[],[],[],[],[],[],
        [\<funcReturnType\>],[15],[],[15],[],[],[],[],[14],[],[],
        [\<ifStmt\>],        [],[],[],[],[16],[],[],[],[],[],
        [\<params\>],        [18],[],[],[],[],[18],[],[],[],[],
        [\<params_n\>],      [20],[],[],[],[],[20],[],[],[],[19],
        [\<returnExpr\>],    [22],[22],[],[22],[],[],[],[],[],[],
        [\<ifCondition\>],   [],[],[],[],[23],[],[],[],[],[],
        [\<else\>],          [26],[26],[],[26],[],[],[],[],[],[],
        [\<elseIf\>],        [],[],[27],[],[],[],[],[],[],[],
        [\<assignType\>],    [30],[30],[],[30],[],[],[29],[],[30],[],
        [\<assignExpr\>],    [32],[32],[],[32],[],[],[],[],[31],[],
    )
]
#figure(
    kind: table,
    caption: [LL-table 2/2]
)[
    #table(
        columns: (auto,auto,auto,auto,auto,auto,auto,auto,auto,auto,auto,auto),
        fill: (x, y) =>
            if x > 0 and y == 0 { yellow }
            else if x == 0 and y > 0 { aqua }
            else { white }
        ,
        align: (x, y) =>
            if x == 0 and y == 0 { center }
            else if x == 0 { left }
            else { center }
        ,
        [nterm \\ term],     [if],[else],[let],[var],[while],[func],[return],[Data],[DataType],[Op],[ID],
        [\<stmtList\>],      [1],[],[1],[1],[1],[1],[1],[],[],[],[1],
        [\<stmtSeparator\>], [],[],[],[],[],[],[],[],[],[],[],
        [\<stmt\>],          [9],[],[10],[11],[6],[7],[8],[],[],[],[12],
        [\<funcReturnType\>],[],[],[],[],[],[],[],[],[],[],[],
        [\<ifStmt\>],        [],[],[16],[],[],[],[],[],[],[],[],
        [\<params\>],        [],[],[],[],[],[],[],[],[],[],[17],
        [\<params_n\>],      [],[],[],[],[],[],[],[],[],[],[],
        [\<returnExpr\>],    [],[],[],[],[],[],[],[],[],[],[],
        [\<ifCondition\>],   [],[],[24],[],[],[],[],[],[],[],[],
        [\<else\>],          [26],[25],[26],[26],[26],[26],[26],[],[],[],[26],
        [\<elseIf\>],        [28],[],[],[],[],[],[],[],[],[],[],
        [\<assignType\>],    [],[],[],[],[],[],[],[],[],[],[],
        [\<assignExpr\>],    [],[],[],[],[],[],[],[],[],[],[],
    )
]
_Note: The table does not include the non-terminal `<expr>` as it is part of the precedence analysis. This means there would be no rule for it in the table._

The table was generated using a program written by team member _Jakub Kloub_, and is available in our GitHub repository in the _ll_table_ branch. The program implements algorithms from the lectures and outputs the ll-table to stdout.

#pagebreak()
