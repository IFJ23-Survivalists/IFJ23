= Lexical Analysis <lexical_analysis>

The implementation for lexical analysis can be found in `scanner.c` files. They come with a set of functions to tokenizize the given input stream, exported in the header file `scanner.h`.

- *`scanner_init(FILE *file)`*:
    This function initializes the scanner with the provided source file, allowing it
    to prepare for the analysis of the source `file`.

- *`scanner_free()`*:
    Upon completion of the scanning process, call this function to release
    any resources allocated for the scanner, such as closing the source file,
    ensuring efficient memory management.

- *`scanner_reset_to_beginning()`*:
    As the name suggests, this function resets the scanner to its initial position in
    the input source, facilitating the possibility of rescanning the input or
    starting a new parsing session.

- *`scanner_advance()` and `scanner_advance_non_whitespace()`*:
    These functions are the core components of our lexical analysis process.
    While both functions advance the scanner to recognize the next token,
    `scanner_advance_non_whitespace()` specifically ignores any whitespace characters,
    allowing us to focus solely on the meaningful tokens within the source code.
    On the other hand, `scanner_advance()` considers all characters, including whitespace.
    This versatility enables us to handle various scenarios, especially when dealing with
    specific operations that require whitespace considerations, such as binary operations.

The scanner operates by reading the source file _character by character_.
This design allows for versatility in input sources, as the source file can be read
not only from a file on disk but also directly from the standard input stream (stdin).

Each time a token is identified, it's saved in a _dynamic-size buffer_ so we can use it
later when necessary, like when we want to re-scan from the start using the
`scanner_reset_to_beginning` function. This buffer is freed up when we're done with the
scanning process and call the `scanner_free` function. The implementation of buffer can be found
in the `scanner.c` file.

Both `scanner_advance` and `scanner_advance_non_whitespace` are equipped to handle errors,
ensuring the appropriate setting of the `LexicalError` or `InternalError` flags in case of
any issues during the tokenization process.

#pagebreak()
== Diagram Deterministic Finite Automata
The implementation of our scanner functions aligns precisely with the deterministic finite automata (DFA) on this page.

#figure(
  image("img/ifj23-diagram.png", width: 100%),
  caption: [Lexical Analysis - Deterministic Finite Automata],
) <diagram>

#pagebreak()

== State Names
\
S Start

#box(
    height: 150pt,
    width: 109%,
    columns(3, gutter: 5pt)[
          F1 DoubleColon
        \ F2 BracketRight
        \ F3 BracketLeft
        \ F4 ParenRight
        \ F5 ParenLeft
        \ F6 Multiply
        \ F7 Plus
        \ F8 Minus
        \ F9 Divide
        \ F10 EqualSign
        \ F11 DoubleEqualSign
        \ F12 LessThan
        \ F13 LessOrEqual
        \ F14 MoreThan
        \ F15 MoreOrEqual
        \ F16 DoubleQuestionMark
        \ F17 Whitespace
        \ F18 Identifier
        \ F19 MaybeNilType
        \ F20 Number
        \ F21 NumberDouble
        \ F22 NumberExponent
        \ F23 StringEnd
        \ F24 BlockCommentStart
        \ F25 Negation
        \ F26 NotEqual
        \ F27 Or
        \ F26 And
    ]
)

#box(
    height: 130pt,
    width: 109%,
    columns(3, gutter: 5pt)[
          Q1 LineComment
        \ Q2 QuestionMark
        \ Q3 NumberDoubleStart
        \ Q4 NumberExponentStart
        \ Q5 NumberExponentSign
        \ Q6 StringStart
        \ Q7 LineString
        \ Q8 LineStringEscape
        \ Q9 LineStringEscapeUnicode
        \ Q10 LineStringEscapeHexStart
        \ Q11 LineStringEscapeHex1
        \ Q12 LineStringEscapeHex2
        \ Q13 DoubleQuote
        \ Q14 BlockStringString
        \ Q15 BlockString
        \ Q16 BlockStringEnd1
        \ Q17 BlockStringEnd2
        \ Q18 BlockStringEnd3
        \ Q19 BlockStringEscape
        \ Q20 BlockStringEscapeUnicode
        \ Q21 BlockStringEscapeHexStart
        \ Q22 BlockStringEscapeHex1
        \ Q23 BlockStringEscapeHex2
        \ Q24 Pipe
        \ Q25 Ampersand
    ]
)

== Note

- *F17 Whitespace*: Combines multiple adjacent whitespaces into one. It also has an attribute (has_eol) to check for End-of-Line (EOL) characters inside the whitespaces.

- *F24 BlockCommentStart*: Whenever the scanner enters this state, it transitions into _Block Comment Mode_ due to the limitation of deterministic finite automata in detecting matching open and close states. In Block Comment Mode:
    - Set internal counter to 1.
    - Utilizes the automata for block comments instead of the one used above.
    - When it encounters *F1 BlockCommentStart*: increase the counter by 1.
    - When it encounters *F2 BlockCommentEnd*: decrease the counter by 1.
    - Returns to normal mode when either counter is 0 or when in *F3 EOF* state.

#figure(
    grid(
        columns: (1.3fr, 1fr),
        gutter: 20pt,
        image("img/ifj23-block_comment_diagram.png", width: 80%),
        [
            #v(50pt)
            #align(left, grid(columns: (1fr, 2fr), gutter: 5pt)[
                  S Start
                \ Q1 Divide
                \ Q2 Multiply
            ][
                  F1 BlockCommentStart
                \ F2 BlockCommentEnd
                \ F3 EOF
            ])
        ]
    ),
    caption: [Block Comment - Deterministic Finite Automata],
)
