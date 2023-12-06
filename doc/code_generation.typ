= Code Generation <code_generation>
Code generation is a crucial component of a compiler or interpreter for a language, providing an abstraction for generating intermediate code from high-level language constructs.

It allows for the separation of concerns between the front-end and the back-end of a compiler, ensuring that modifications or adaptations in the front-end components do not necessitate
extensive changes to the code generation logic.

== Instructions <instructions>
We included three address instructions and stack instructions into the system architecture.\
Sprecifically three address instructions follow a structured format comprising of a singular instruction code followed by three addresses, adhering to this predefined structure:\ \
*`OPCODE DESTINATION SRC1 SRC2`* \ \
In the prescribed format, the opcode denotes the specific instruction, while the destination designates the target variable where the outcome of the operation is stored. The two sources represent the operands involved in the operation. \

_Note: Not all instructions necessitate both sources, in such instances, the corresponding fields are designated as NULL_

== Implementation

The implementation incorporates a flexible design to accommodate various operand types, such as variables, symbols, data types and labels.
Additionally, the code generator manages different frames (Global, Local, Temporary), ensuring proper scoping and variable resolution during code execution.\

The primary data structure, CodeBuf, is used to manage the generated instructions. The code_buf_init function initializes this buffer with an initial capacity, and the code_buf_push function appends generated instruction strings to it. The module supports dynamic resizing of the buffer when its capacity is exhausted. Additionally, error handling is incorporated throughout the code, with the set_error function being called in case of memory allocation failures.

The code employs a set of macros, such as push_instruction and push_var, to facilitate the generation of different types of instructions.\ These macros are used within the code_generation function, which takes high-level instructions, operands, and translates them into the IFJcode23 code. The code also includes utilities for encoding strings and literals, and it handles different data types appropriately.\ The modular structure of the code allows for easy extension and modification to accommodate new instructions or language features.

\ Overall, the code provides a foundation for translating abstract language constructs into executable machine code.
