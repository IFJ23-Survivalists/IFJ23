= Lexical Analysis

In the context of our Swift programming language interpreter,
the process of lexical analysis plays a fundamental role in parsing the source code.
This initial phase of the interpreter involves breaking down the input source file
into a sequence of tokens. To achieve this, we have implemented a set of functions
in the `scanner.h` header file.

- *`scanner_init(FILE *file)`*:
    This function initializes the scanner with the provided source file, allowing it
    to prepare for the analysis of the source `file`.

- *`scanner_free()`*:
    Upon completion of the scanning process, this function is responsible for releasing
    any resources allocated for the scanner, such as closing the source file,
    ensuring efficient memory management.

- *`scanner_reset_to_beginning`*:
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

#figure(
  image("img/ifj23-text.png", width: 100%),
)

#lorem(60)

#figure(
  image("img/ifj23-block_comment_diagram.png", width: 100%),
  caption: [Block Comment - Deterministic Finite Automata],
)

#lorem(60)
