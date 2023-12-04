/**
 * @file codegen.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/12/2023
 * @brief Header file for code generation API
 */

#ifndef H_CODEGEN
#define H_CODEGEN

#include "scanner.h"

typedef enum {
    /// The first and must-have instruction
    /// Params: None
    Instruction_Start,

    /// Copy the value of ⟨symb⟩ to ⟨var⟩. For example, MOVE LF@par GF@var copies the value
    /// of the variable var in the global frame to the variable par in the local frame.
    /// Params: var, symb
    Instruction_Move,

    /// Create a new temporary frame and discard the content of the current temporary frame.
    /// Params: None
    Instruction_CreateFrame,

    /// Move the temporary frame to the stack of frames. The frame will be available through LF
    /// and will overlay the original frames on the stack of frames. TF will be undefined after
    /// executing this instruction, and it needs to be created again using CREATEFRAME before
    /// further use. Accessing an undefined frame leads to error 55.
    /// Params: None
    Instruction_PushFrame,

    /// Move the top frame LF from the stack of frames to TF. If no frame in LF is available,
    /// it leads to error 55.
    /// Params: None
    Instruction_PopFrame,

    /// Define a variable in the specified frame according to ⟨var⟩. This variable is initially
    /// uninitialized and without specifying a type, which will be determined only when assigning a value.
    /// Params: var
    Instruction_DefVar,

    /// Save the incremented current position from the internal instruction counter to the call stack
    /// and jump to the specified label (any frame preparation must be ensured by other instructions).
    /// Params: label
    Instruction_Call,

    /// Take the position from the call stack and jump to this position by setting the internal instruction
    /// counter (local frame cleanup must be ensured by other instructions). Executing the instruction with
    /// an empty call stack leads to error 56.
    /// Params: None
    Instruction_Return,

    /// Save the value ⟨symb⟩ on the data stack
    /// Params: symb
    Instruction_Pushs,

    /// If the stack is not empty, take a value from it and save it to the variable ⟨var⟩; otherwise,
    /// it leads to error 56.
    /// Params: var
    Instruction_Pops,

    /// Helper instruction that clears the entire content of the data stack to avoid forgotten values from
    /// previous computations.
    /// Params: None
    Instruction_Clears,

    /// Arithmetic operations ⟨symb1⟩ and ⟨symb2⟩ (must be of the same numeric type int or float),
    /// and the resulting value of the same type is stored in the variable ⟨var⟩.
    /// Params: var, symb1, symb2
    Instruction_Add,
    Instruction_Sub,
    Instruction_Mul,
    Instruction_Div,
    Instruction_Idiv,

    /// Stack versions
    /// Params: None
    Instruction_Adds,
    Instruction_Subs,
    Instruction_Muls,
    Instruction_Divs,
    Instruction_Idivs,

    /// Conditional
    /// Params: var, symb1, symb2
    Instruction_Lt,
    Instruction_Gt,
    Instruction_Eq,

    /// Stack versions
    /// Params: None
    Instruction_Lts,
    Instruction_Gts,
    Instruction_Eqs,

    /// Params: var, symb1, symb2
    Instruction_And,
    Instruction_Or,
    Instruction_Not,

    /// Stack versions
    /// Params: None
    Instruction_Ands,
    Instruction_Ors,
    Instruction_Nots,

    /// Conversion
    /// Params: var, symb1
    Instruction_Int2Float,
    Instruction_Float2Int,
    Instruction_Int2Char,
    Instruction_Stri2Int,

    /// Stack versions
    /// Params: None
    Instruction_Int2Floats,
    Instruction_Float2Ints,
    Instruction_Int2Chars,
    Instruction_Stri2Ints,

    /// Read one value according to the specified type ⟨type⟩ ∈ {int, float, string, bool} (including
    /// possible conversion of the input value to float when the type is int) and save this value to the
    /// variable ⟨var⟩. The value format is compatible with the behavior of built-in functions readString,
    /// readInt, and readDouble in the IFJ23 language.
    /// Params: var, type
    Instruction_Read,

    /// Print the value ⟨symb⟩ to the standard output. The output format is compatible with the built-in
    /// write command in the IFJ23 language, including the output of decimal numbers using the format string "%a".
    /// Params: symb
    Instruction_Write,

    /// String operation
    /// Params: var, symb1, symb2
    Instruction_Concat,
    /// Params: var, symb
    Instruction_Strlen,

    /// Params: var, symb1, symb2
    Instruction_GetChar,
    Instruction_SetChar,

    /// Params: var, symb
    Instruction_Type,

    /// Jumps
    /// Params: label
    Instruction_Label,

    /// Params: label
    Instruction_Jump,

    /// Conditional jumps
    /// Params: label, symb1, symb2
    Instruction_JumpIfEq,
    Instruction_JumpIfNeq,

    /// Stack versions
    /// Params: None
    Instruction_JumpIfEqs,
    Instruction_JumpIfNeqs,

    /// End the program execution and terminate the interpreter with the return code ⟨symb⟩, where
    /// ⟨symb⟩ is an integer in the range 0 to 49 (inclusive). An invalid integer value ⟨symb⟩ leads to
    /// error 57.
    /// Params: symb
    Instruction_Exit,

    /// Print the interpreter state at the moment of executing this instruction to the standard error output
    /// (stderr). The state includes the code position, the output of the global, current local, and temporary
    /// frame, and the number of instructions already executed.
    /// Params: None
    Instruction_Break,

    /// Print the specified value ⟨symb⟩ to the standard error output (stderr). Outputs by this instruction
    /// can be turned off using interpreter options (see interpreter help).
    /// Params: symb
    Instruction_DebugPrint,
} Instruction;

/**
 * @brief Enumeration representing different frames for the custom interpreter.
 */
typedef enum {
    Frame_Global, ///< Global Frame (GF)
    Frame_Local, ///< Local Frame (LF)
    Frame_Temporary, ///< Temporary Frame (TF)
} Frame;

/**
 * @brief Structure representing a variable with its frame and name.
 */
typedef struct {
    Frame frame; ///< Frame of the variable
    char *name;  ///< Name of the variable
} Variable;

/**
 * @brief Enumeration representing the type of a symbol (Variable or Constant).
 */
typedef enum {
   SymbolType_Variable, ///< Symbol represents a variable
   SymbolType_Constant, ///< Symbol represents a constant
} SymbolType;

/**
 * @brief A tagged union representing either a Variable or a Constant.
 */
typedef struct {
    SymbolType type; ///< Type of the union

    union {
        Variable variable; ///< Represents a variable
        Data constant; ///< Represents a constant
    };
} Symbol;

/**
 * @brief Union representing different operand types (Variable, Symbol, DataType, Label).
 */
typedef union {
    Variable variable; ///< Represents a variable
    Symbol symbol;     ///< Represents a symbol
    DataType data_type; ///< Represents a data type
    char *label;       ///< Represents a label
} Operand;
/**
 * @brief Structure representing a generated instruction.
 */
typedef struct {
    String code; ///< The generated code for the instruction
} GeneratedInstruction;

/**
 * @brief Structure representing a buffer for storing generated code instructions.
 */
typedef struct {
    GeneratedInstruction *buf; ///< Buffer storing generated instructions
    size_t size;               ///< Current size of the buffer
    size_t capacity;           ///< Capacity of the buffer
} CodeBuf;

/**
 * @brief Initialize a CodeBuf instance.
 * @param buf The CodeBuf instance to initialize.
 */
void code_buf_init(CodeBuf *buf);

/**
 * @brief Free the resources associated with a CodeBuf instance.
 * @param buf The CodeBuf instance to free.
 */
void code_buf_free(CodeBuf *buf);

/**
 * @brief Set the active `CodeBuf`.
 * @param buf The CodeBuf to set as active.
 */
void code_buf_set(CodeBuf *buf);

/**
 * @brief Print all instructions in the active `CodeBuf` to stdout.
 * @param buf The `CodeBuf` to be printed.
 */
void code_buf_print(CodeBuf *buf);

/**
 * @brief Print all instructions in the active `CodeBuf` to a `String` and return it.
 * @param buf The `CodeBuf` to be printed.
 * @return instructions `String`
 */
String code_buf_print_to_string(CodeBuf *buf);

/**
 * @brief Generate code for an instruction with specified operands and insert it into the active `CodeBuf`.
 * @param instruction The instruction to generate code for.
 * @param operand1 The first operand.
 * @param operand2 The second operand.
 * @param operand3 The third operand.
 */
void code_generation(Instruction, Operand *, Operand *, Operand *);

/**
 * @brief Generate raw code right into the code buffer without any checks.
 * @param code Code to output.
 */
void code_generation_raw(const char* code);
#endif
