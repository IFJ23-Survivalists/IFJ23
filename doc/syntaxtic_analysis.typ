= Syntaktická analýza <syntaxtic_analysis>
Syntaktickou analýzu jsme se rozhodli implementovat pomocí rekurivního sestupu a precedenční analýzy.

== LL-gramatika
List pravidel vypadá následovně:
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

Gramatika obsahuje několik nejasností, které jsou řešeny až v kódu:
- `<expr>` - Pokud se použije pravidlo, které obsahuje tento neterminál, tak se přepneme do precenedční analýzy.
- `<stmtSeparator>` - Konkrétně se jedná o pravidlo č. 4, které když se použije, tak se token '`}`' nevyjme ze vstupní pásky. Důvodem je, že toto pravidlo existuje pouze, aby konec kódových bloků mohl oddělovat výrazy bez potřeby toho, aby mezi nima byl znak konce řádku. Zde ale nechceme zkonzumomvat konece bloku, abychom jsme ho mohli zpracovat až v pravidlech `6`, `7` atd.
- Pravidlo `12` vs `13`:
    - Zde se rozhodujem na základně toho, jestli `ID` je identifikátor funkce či proměnné. Pokud je to funkce, tak se přepneme do precedenční analýzy s použitím pravidla `13`, jinak předpokládáme, že se jedná o přiřazení a použijeme pravidlo `12`.
    - Nevýhoda tohoto přístupu je, že nemáme jak podporovat výrazy typu: ```swift
        var a = 0
        a + 1
    ``` kde se ale jedná o mrtvý kód, takže jsme se rozhodli, že to nepotřebujeme podporovat.
- Token `EOL` - scanner jej počítá jako bílý znak společně s komentáři. Způsob jakým ho rozlišujeme je, že tokeny bílých znaků obsahuje atribut, který říká jestli obsahuje konec řádku či nikoliv.

== LL-tabulka
Z pravidel je vytvořená následující tabulka:
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
_Pozn.: V tabulce není zaznačen neterminál `<expr>`, protože ten je součástí precedenční analýzy. To znamená, že by pro něj nebylo v tabulce zaznačené žádné pravidlo._

Tabulka byla vygenerována pomocí programu, který napsal člen týmu _Jakub Kloub_ a je dostupný v našem Github repozitáři ve větvi _ll_table_. Program pouze implementuje algoritmy z přednášek a vypisuje tabulku.

#pagebreak()
