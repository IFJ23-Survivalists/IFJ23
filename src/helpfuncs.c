// DONE: func readString() -> String? = 
// DONE: func readInt() -> Int?
// DONE: func readDouble() -> Double?
// DONE: func readBool() -> Bool?

// DONE: func write(term)           :) enjoy DONE with unlimited terms

// DONE: func Int2Double(_ term : Int) -> Double
// DONE: func Int2Bool(_ term : Int) -> Bool
// DONE: func Bool2Int(_ term : Bool) -> Int

// DONE: func length(_ s : String) -> Int
// DONE: func substring(of s : String, startingAt i : Int, endingBefore j : Int) -> String?
// DONE: func ord(_ c : String) -> Int
// DONE: func chr(_ i : Int) -> String



// InProgress: func readString() -> String?

FunctionSymbol rString;
function_symbol_init(&rString);

rString.return_value_type = DataType_MaybeString;

parser_parameter_code_infos(&rString);
parser_function_code_info(&rString, "reads");
/*CODE_GENERATION*/
code_buf_set(&rString.code);



code_buf_set(&g_parser.global_code);
symtable_insert_function(symstack_bottom(),"reads", rString );


/*
LABEL reads:

\tCREATEFRAME\n
\tPUSHFRAME\n,

\tREAD GF@?tmp string\n
\tPUSHS GF@?tmp\n

\tPOPFRAME\n
\tRETURN\n

*/
/*---------------------------------------------------*/
/*
LABEL readint:

\tCREATEFRAME\n
\tPUSHFRAME\n,

\tREAD GF@?tmp int\n
\tPUSHS GF@?tmp\n

\tPOPFRAME\n
\tRETURN\n

*/
/*---------------------------------------------------*/
/*

LABEL readdouble:

\tCREATEFRAME\n
\tPUSHFRAME\n,

\tREAD GF@?tmp float\n
\tPUSHS GF@?tmp\n

\tJUMP ?func_end\n

*/
/*---------------------------------------------------*/
/*

LABEL readbool:

\tCREATEFRAME\n
\tPUSHFRAME\n,

\tREAD GF@?tmp bool\n           #define global helping variable tmp
\tPUSHS GF@?tmp\n

\tJUMP ?func_end\n

*/
/*---------------------------------------------------*/
/*

LABEL: write

\tCREATEFRAME\n
\tPUSHFRAME\n,

\tDEFVAR LF@wrcycle\n
\tPOPS LF@wrcycle\n
\tLABEL write&while\n

\tJUMPIFEQ ?func_end LF@wrcycle int@0\n  #ends if there's nothing more to write.
\tPOPS GF@?tmp\n

\tWRITE GF@?tmp\n
\tSUB LF@wrcycle LF@wrcycle int@1\n
\tJUMP write&while 


*/
/*---------------------------------------------------*/
/*

LABEL: ?func_end #ends function and returns.

\tPOPFRAME\n
\tRETURN\n
\n

*/
/*---------------------------------------------------*/
/*

LABEL: Int2Double

\tCREATEFRAME\n
\tPUSHFRAME\n

\tPOPS GF@?tmp\n
\tPUSHS GF@?tmp\n
\tINT2FLOATS\n
\tJUMP ?func_end\n

*/
/*---------------------------------------------------*/
/*

LABEL: Int2Bool

\tCREATEFRAME\n
\tPUSHFRAME\n

\tPOPS GF@?tmp\n
\tJUMPIFEQ ret_false GF@?tmp int@0\n
\tPUSHS bool@true\n
\tJUMP ?func_end\n
\tLABEL ret_false\n
\tPUSHS bool@false\n
\tJUMP func_end\n

*/
/*---------------------------------------------------*/
/*

LABEL: Bool2Int

\tCREATEFRAME\n
\tPUSHFRAME\n

\tPOPS GF@?tmp
\tJUMPIFEQ int_zero GF@?tmp bool@false\n
\tPUSHS int@1\n
\tJUMP ?func_end
\tLABEL int_zero\n
\tPUSHS int@0\n
\tJUMP ?func_end\n

*/
/*---------------------------------------------------*/
/*

LABEL: length

\tCREATEFRAME\n
\tPUSHFRAME\n

\tDEFVAR LF@$s\n
\tPOPS LF@$s\n
\tPUSHS LF@$s\n
\tCALL ?type_str\n
\tSTRLEN LF@$s LF@$s\n
\tPUSHS LF@$s\n
\tJUMP ?fun_end\n

*/
/*---------------------------------------------------*/
/*

LABEL:substring

\tCREATEFRAME\n
\tPUSHFRAME\n

\tDEFVAR LF@$s\n
\tPOPS LF@$s\n
\tPUSHS LF@$s\n
\tCALL ?type_str\n
\tDEFVAR LF@$i\n
\tPOPS LF@$i\n
\tPUSHS LF@$i\n
\tCALL ?type_int\n
\tDEFVAR LF@$j\n
\tPOPS LF@$j\n
\tPUSHS LF@$j\n
\tCALL ?type_int\n
\tLTS GF@?tmp LF@$i int@0\n
\tJUMPIFEQ substring&null GF@?tmp bool@true\n

\tLTS GF@?tmp LF@$j int@0\n
\tJUMPIFEQ substring&null GF@?tmp bool@true\n

\tGTS GF@?tmp LF@$i LF@$j\n
\tJUMPIFEQ substring&null GF@?tmp bool@true\n

\tPUSHS LF@$s\n
\tCALL strlen\n
\tPOPS GF@?aux\n

\tLTS GF@?tmp LF@$i GF@?aux\n
\tJUMPIFNEQ substring&null GF@?tmp bool@true\n

\tGTS GF@?tmp LF@$j GF@?aux\n
\tJUMPIFEQ substring&null GF@?tmp bool@true\n

\tMOVE GF@?tmp string@\n
\tLABEL substring&while_start\n
\tJUMPIFEQ substring&while_end LF@$i LF@$j\n
\tGETCHAR GF@?aux LF@$s LF@$i\n
\tCONCAT GF@?tmp GF@?tmp GF@?aux\n
\tADD LF@$i LF@$i int@1\n
\tJUMP substring&while_start\n
\tLABEL substring&while_end\n
\tPUSHS GF@?tmp\n
\tJUMP ?fun_end\n
\tLABEL substring&null\n
\tPUSHS nil@nil\n
\tJUMP ?fun_end\n

*/
/*---------------------------------------------------*/
/*

LABEL:ord

\tCREATEFRAME\n
\tPUSHFRAME\n

\tDEFVAR LF@$c\n
\tPOPS LF@$c\n
\tPUSHS LF@$c\n
\tCALL ?type_str\n
\tPUSHS LF@$c\n
\tCALL strlen\n
\tPOPS GF@?tmp\n

\tJUMPIFEQ ord&empty_string GF@?tmp int@0\n

\tPUSHS LF@$c\n
\tPUSHS int@0\n
\tSTRI2INTS\n
\tJUMP ?fun_end\n
\tLABEL ord&empty_string\n
\tPUSHS int@0\n
\tJUMP ?fun_end\n

*/
/*---------------------------------------------------*/
/*

LABEL:chr

\tCREATEFRAME\n
\tPUSHFRAME\n

\tDEFVAR LF@$i\n
\tPOPS LF@$i\n
\tPUSHS LF@$i\n
\tCALL ?type_int\n
\tPUSHS LF@$i\n
\tINT2CHARS\n
\tJUMP ?fun_end\n
*/








/*
// TODO: func substring(of s : String, startingAt i : Int, endingBefore j : Int) -> String?

FunctionSymbol func
function_symbol_init(&func);
func.return_value_type = DataType_MaybeString;
function_symbol_emplace_param(&func, DataType_String, "of", "s");
function_symbol_emplace_param(&func, DataType_Int, "startingAt", "i");
function_symbol_emplace_param(&func, DataType_Int, "endingBefore", "j");
parser_parameter_code_infos(&func);
parser_function_code_info(&func, "substring");

// Code generation
code_buf_set(&func.code);       // Generate code into this function.
// PUSHFRAME
// ...
// 's' je v prom. 'LF@s%0'
// 'i' je v prom. 'LF@i%1'
// 'j' je v prom. 'LF@j%2'
// ...
// CREATEFRAME
// DEFVAR TF@ret
// MOVE TF@ret string@"return value"
// POPFRAME
// RETURN
code_buf_set(&g_parser.global_code);
symtable_insert_function(symstack_bottom(), "substring", func);
*/