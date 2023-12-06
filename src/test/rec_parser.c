/**
 * @note Project: Implementace překladače imperativního jazyka IFJ23
 * @file test/rec_parser.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 29/11/2023
 * @brief Tester for rec_parser.c.
 */

#include "../parser.h"
#include "../scanner.h"
#include "test.h"

int prg_scanner_initialized() {
    scanner_reset_to_beginning();
    MASSERT(!got_error(), "");
    parser_init();
    MASSERT(!got_error(), "");

    parser_begin(false);
    int res = (int)got_error();
    if (res != 0) {
        if (got_int_error()) {
            print_int_error_msg();
            clear_int_error();
        }
        print_error_msg();
        set_error(Error_None);
    }

    parser_free();
    MASSERT(!got_error(), "");
    scanner_free();
    MASSERT(!got_error(), "");
    return res;
}

int prg(const char* source_code) {
    scanner_init_str(source_code);
    MASSERT(!got_error(), "");
    return prg_scanner_initialized();
}
int prg_file(const char* file_path) {
    FILE* f = fopen(file_path, "r");
    MASSERT(f != NULL, "");
    scanner_init(f);
    MASSERT(!got_error(), "");
    return prg_scanner_initialized();
}

int main() {
    atexit(summary);

    suite("Test Parser syntax/semantics - Let statements - correct") {
        set_print_errors(true);
        test(prg("let a = 0") == 0);

        test(prg("let a = 0 \n let b : String = \"hello\"") == 0);
        test(prg("let a = 0 \n let b : String \n = \"hello\" \n let c : Int = 12") == 0);
        test(prg("let a = 0 \n let b : String = \"hello\"") == 0);

        test(prg("let b : Int? = 123") == 0);
        test(prg("let b : String? = \":D\"") == 0);
        test(prg("let b : Bool? = false") == 0);
        test(prg("let b : Double? = 3.14159") == 0);

        test(prg("let b : Int? = nil") == 0);
        test(prg("let b : String? = nil") == 0);
        test(prg("let b : Bool? = nil") == 0);
        test(prg("let b : Double? = nil") == 0);

        test(prg("let a = 123\nlet b: Int? = a") == 0);
        test(prg("let a : Int? = 123\nlet b = a!\nlet c : Int = b") == 0);
    }
    suite("Test Parser syntax/semantics - Let statements - invalid") {
        set_print_errors(false);
        test(prg("let a : Int = \"fuj fuj\"") == 7);
        test(prg("let a = 0\nlet a = 2") == 3);
        test(prg("let a = 0\nlet a = \":)\"") == 3);
        test(prg("let a = 0\n a = 1") == 3);
        test(prg("let a = nil") == 8);
        test(prg("let a : Int = nil") == 7);
        test(prg("let a : String = 0.0") == 7);
        test(prg("let a: Int? = 1\nlet b : Int = a") == 7);
        test(prg("let a = 123\n let b : String = a\n") == 7);
        test(prg("let = 12") == 2);
        test(prg("let a") == 2);
        test(prg("let a : Int") == 2);
        test(prg("let a : Int?") == 2);
        test(prg("let := 12") == 2);
        test(prg("let : Int 12") == 2);
        test(prg("let a : Int 12") == 2);
        test(prg("let Int : Int = 12") == 2);
        test(prg("let Int = 12") == 2);
        test(prg("let = 12") == 2);
        test(prg("let 12") == 2);
    }
    suite("Test Parser syntax/semantics - Var statements - correct") {
        set_print_errors(true);

        test(prg("var a = 0") == 0);
        test(prg("var a\n") == 0);

        test(prg("var a = 1") == 0);
        test(prg("var a = 3.145") == 0);
        test(prg("var a = \"this is a test\"") == 0);
        test(prg("var a = true") == 0);
        test(prg("var a = false") == 0);

        test(prg("var a\na = 1\nvar b : Int = a") == 0);
        test(prg("var a\na = 3.145\nvar b : Double = a") == 0);
        test(prg("var a\na = \"this is a test\"\nvar b : String = a") == 0);
        test(prg("var a\na = true\nvar b : Bool = a") == 0);

        test(prg("var a : Int\na = 1\nvar b : Int = a") == 0);
        test(prg("var a : Double\na = 3.145\nvar b : Double = a") == 0);
        test(prg("var a : String\na = \"this is a test\"\nvar b : String = a") == 0);
        test(prg("var a : Bool\na = true\nvar b : Bool = a") == 0);

        test(prg("var a = 0 \n var b : String = \"hello\"") == 0);
        test(prg("var a = 0 \n var b : String \n = \"hello\" \n var c : Int = 12") == 0);
        test(prg("var a = 0 \n var b : String = \"hello\"") == 0);

        test(prg("var b : Int? = 123") == 0);
        test(prg("var b : String? = \":D\"") == 0);
        test(prg("var b : Bool? = false") == 0);
        test(prg("var b : Double? = 3.14159") == 0);

        test(prg("var b : Int? = nil") == 0);
        test(prg("var b : String? = nil") == 0);
        test(prg("var b : Bool? = nil") == 0);
        test(prg("var b : Double? = nil") == 0);

        test(prg("var a = 123\nlet b: Int? = a") == 0);
        test(prg("var a : Int? = 123\nlet b = a!\nlet c : Int = b") == 0);
    }
    suite("Test Parser syntax/semantics - Var statements - invalid") {
        set_print_errors(false);
        test(prg("var a : Int = \"fuj fuj\"") == 7);
        test(prg("var a = 0\nlet a = 2") == 3);
        test(prg("var a = 0\nlet a = \":)\"") == 3);
        test(prg("var a = 0\n a = nil") == 7);
        test(prg("var a = nil") == 8);
        test(prg("var a\na = nil") == 8);
        test(prg("var a : Int = nil") == 7);
        test(prg("var a : String = 0.0") == 7);
        test(prg("var a: Int? = 1\nlet b : Int = a") == 7);
        test(prg("var a = 123\n var b : String = a\n") == 7);
        test(prg("var = 12") == 2);
        test(prg("var := 12") == 2);
        test(prg("var : Int 12") == 2);
        test(prg("var a : Int 12") == 2);
        test(prg("var Int : Int = 12") == 2);
        test(prg("var Int = 12") == 2);
        test(prg("var = 12") == 2);
        test(prg("var 12") == 2);
    }
    suite("Test Parser syntax/semantics - Assign statements - correct") {
        set_print_errors(true);
        test(prg("let a = 12\n var b = 31\n b = a + b\nvar c = a") == 0);
        test(prg("var a: Int? = 41\n var b = a! + 6\n a = a ?? 6") == 0);
        test(prg("var a = 12\nvar b : Int? = 13\na = a - (5 * b!)") == 0);
        test(prg("var a\n var b\n a = 12\n b = 13\n let c : Int = a + b") == 0);
        test(prg("var a\n var b\n a = 12\n b = 13\n let c : Int? = a + b") == 0);
        test(prg("var a\n func foo() -> Int { return 0 } a = foo() + 1\nlet c : Int = a") == 0);
    }
    suite("Test Parser syntax/semantics - Assign statements - invalid") {
        set_print_errors(false);
        test(prg("var a : Int = 1\n var b : String = \"ahoj\"\n b = a + b") == 7);
        test(prg("var a : Int? = nil\n a = 3.14") == 7);
        test(prg("a = 123") == 5);
        test(prg("var a \n a = a + b * 1") == 5);
        test(prg("if (1 == 1) { var a = 14 } a = 15") == 5);
        test(prg("var a \n a = \nlet c = 2") == 2);
        test(prg("var a \n a = \nvar c = 2") == 2);
        test(prg("if (1 == 1) { var a \n a = }") == 2);
        test(prg("if (1 == 1) { var a \n a = \n}") == 2);
    }
    suite("Test Parser syntax/semantics - If/If-let statements - correct") {
        set_print_errors(true);
        test(prg("if ( 1== 0) { let c = 0 }") == 0);
        test(prg("if (2 == 2) {}") == 0);
        test(prg("if\n\n (\n 1\n== 0\n)\n { let c = 0 }") == 0);
        test(prg("let a : Int = 0\nif(a == 1) { let c = 0\nvar d = 2}") == 0);
        test(prg("var a\na= true\n if (a) { let c = 0 }") == 0);
        test(prg("let a= false\n if (a || true) { let c = 0 }") == 0);
        test(prg("if 1 == 0 { let c = 0 }") == 0);

        test(prg("let a : Int? = nil\n if let a { var c : Int = 0\n c = c + a}") == 0);
        test(prg("let a : Int = \n10\n if let a { var c : Int = 0\n c = c + a}") == 0);
        test(prg("let a = 12\n if let a { var c : Int = 0\n c = c + a}") == 0);
        test(prg("let a : Int? = nil\n if\n let a\n {\n var c : Int = 0\n c = c + a\n}\n") == 0);
        test(prg("var a = 0\n"
                 "if (a == 0) {\n"
                 "var a = 0.0\n"
                 "} else if (a == 1) {\n"
                 "var b = 0.0\n"
                 "} else {\n"
                 "a = a+ 1\n"
                 "}\n"
                 "var b = 0\n") == 0);
    }
    suite("Test Parser syntax/semantics - If/If-let statements - errors") {
        set_print_errors(false);
        test(prg("if(){}") == 2);
        test(prg("if (1 + 0) { let c = 0 }") == 7);
        test(prg("let a= \"aalkdjf\"\n if a { let c = 0 }") == 7);
        test(prg("let a= \"aalkdjf\"\n if (a) { let c = 0 }") == 7);
        test(prg("if let (a == 0) { let c = 0 }") == 2);
        test(prg("let a : Int? =0\nif let a { let a = 0 }") == 3);
        test(prg("var a : Int? =0\nif let a { let a = 0 }") == 7);
        test(prg("if let a { let c = 0 }") == 5);
        test(prg("var a\nif let a { let c = 0 }") == 5);
    }
    suite("Test Parser syntax/semantics - While statements - correct") {
        set_print_errors(true);
        test(prg("while(true){}") == 0);
        test(prg("while(1 == 1){}") == 0);
        test(prg("while (12 == 12) { let c = 0\nvar d= 0\n if (c == d) { let h = 0 }}") == 0);
        test(prg("while ((12 + 15) == 10) { let a = 0 }") == 0);
        test(prg("let a: Bool = false\nwhile (a) { let a = 0 }") == 0);
        test(prg("let a: Bool? = nil\nwhile (a??true) { let a = 0 }") == 0);
        test(prg("let a: Bool? = nil\nwhile (a!) { let a = 0 }") == 0);
        test(prg("var a\na = 1 == 1\nwhile(a) { let c = 0 }") == 0);
        test(prg("var a\na = 1 == 1\nwhile(a) { let c = 0}") == 0);
    }
    suite("Test Parser syntax/semantics - While statements - errors") {
        set_print_errors(false);
        test(prg("while(){}") == 2);
        test(prg("while {}") == 2);
        test(prg("while () let a = 0") == 2);
        test(prg("while(1+1){}") == 7);
        test(prg("let a = 12\nwhile(a){}") == 7);
        test(prg("let a : Int? = 12\nwhile(a!){}") == 7);
        test(prg("var c\nc = \"lul\"\nwhile(c + 1){}") == 7);
        test(prg("var c\nc = \"lul\"\nwhile(c){}") == 7);
        test(prg("while((let a = 0) == 0){\n\n\n\n\n}") == 2);
    }
    suite("Test Parser syntax/semantics - Func statements - correct") {
        set_print_errors(true);
        test(prg("func foo() {}") == 0);
        test(prg("func foo() {\n}") == 0);
        test(prg("func foo() -> Int { return 1 }") == 0);
        test(prg("func foo() -> Int { return 1\n}") == 0);
        test(prg("func foo() -> Int? { return 1}") == 0);
        test(prg("func foo() -> Int? { return 1\n}") == 0);
        test(prg("func foo() -> Int? { return nil}") == 0);
        test(prg("func foo() -> Int? { return nil\n}") == 0);
        test(prg("func foo() { return }") == 0);
        test(prg("func foo() { return\n}") == 0);
        test(prg("func foo(from x : Int, to y : String) {}") == 0);
        test(prg("func foo(from x : Int, to y : String) -> String { return \"ahoj\" }") == 0);
        test(prg("func foo(from x : Int, to y : String) -> String { return \"ahoj\"\n\n}") == 0);
        test(prg("func foo(from x : Int, to y : String) -> String { return y }") == 0);
        test(prg("func foo(from x : String?, to y : String) -> String { return (x ?? \" \") + y }") == 0);
        test(prg("func foo(from x : String?, to y : String) -> String { return (x ?? \" \") + y\n}") == 0);
        test(prg("var a = foo(123)\nfunc foo(_ param: Int) -> Int { return param * param}") == 0);
        test(prg("var a : Int = foo(123) + 12\nfunc foo(_ param: Int) -> Int { return param * param}") == 0);
        test(prg("func foo(_ param: Int) -> Int { return param * param} var a : Int = foo(13)") == 0);
        test(prg("func foo(_ param: Int) -> Int { return param * param}\nvar a : Int = foo(12)") == 0);
        test(prg("func foo(_ param: Int) -> Int { return param * param}\nvar a = foo(-12)\n var b : Int = a") == 0);
        test(prg("foo()\n func foo() {}") == 0);
        test(prg("func foo() {} foo()") == 0);
        test(prg("func foo() {}\n foo()") == 0);
        test(prg("func foo(\n) {}\n foo()") == 0);
        test(prg("func foo() { var a = bar()\n var b : Int = a } func bar() -> Int { return -1 }") == 0);
        test(prg("func foo() { var a = bar()\n var b : Int = a }\nfunc bar() -> Int { return -1 }") == 0);
        test(prg("func bar() -> Int { return -1 } func foo() { var a = bar()\n var b : Int = a }") == 0);
        test(prg("func bar() -> Int { return -1 }\nfunc foo() { var a = bar()\n var b : Int = a }") == 0);
        test(
            prg("func bar() -> Int { return -1 }\n foo() \n bar() \n func foo() { var a = bar()\n var b : Int = a }") ==
            0);
        test(prg("func bar() -> Int { return -1 } foo() \n bar() \n func foo() { var a = bar()\n var b : Int = a }") ==
             0);
    }
    suite("Test Parser syntax/semantics - Func statements - errors") {
        set_print_errors(false);
        test(prg("func bar(from a : Int, to b : Int) { return 1 }") == 6);
        test(prg("func bar(from a : Int, to b : Int) { return nil }") == 6);
        test(prg("func bar(from a : Int, to b : Int) -> Int { }") == 6);
        test(prg("return 1 + 2 \n func bar(from a : Int, to b : Int) -> Int { return 1 }") == 6);
        test(prg("func bar(from a : Int, to b : Int) -> Int { return 1 }\n return 1+ 1") == 6);
        test(prg("func bar(from a : Int, to b : Int) -> Int { return \"lsdkfhh\" }") == 4);
        test(prg("func bar(from a : Int, to b : Int) -> Int? { return \"lsdkfhh\" }") == 4);
        test(prg("func bar(from a : Int, from b : Int) {}") == 9);
        test(prg("func bar(from a : Int, to a : Int) {}") == 9);
        test(prg("func bar(from a : Int, from a : Int) {}") == 9);
        test(prg("func foo(from a : Int, to b : Int) -> String { return a + b}") == 4);
        test(prg("func foo(from a : Int, to b : Int) -> String { return nil}") == 4);
    }
    suite("Test Parser syntax/semantics - Scope") {
        set_print_errors(true);
        test(prg("var a = 0\nif (1 == 1) { var a = \"test\"\n var c : String = a }") == 0);
        test(prg("var a = 0\nwhile (1 == 1) { var a = \"test\"\n var c : String = a }") == 0);
        test(prg("let b : String? = \"adkj\"\nvar a = 0\nif let b { var a = \"test\"\n var c : String = a }") == 0);
        test(prg("var a = 0\nfunc foo() { var a = \"test\"\n var c : String = a }") == 0);
        test(prg("let a : Int? = 12\n if let a { var b = a + 12 }") == 0);
        test(prg("let a = 12\n if (true) { let a = 2.2\n while (true) { let a = false\n var c : Bool = a } var c : "
                 "Double = a } var c : Int = a") == 0);
    }
    suite("Test Parser syntax/semantics - Uninitialized variables") {
        set_print_errors(false);
        test(prg("var a: Int\nif (a == 2) {}") == 5);
        test(prg("var a: Int?\n let c = a! * 2") ==
             0);  // Maybe variables are implicitly initialized to nil on empty initalization.
        test(prg("var a: Int\nwhile (a == 2) {}") == 5);
        test(prg("var a\nwhile (a == 2) {}") == 5);
        test(prg("var a: Int\n func foo() { var b = a * 2 }") == 5);
        test(prg("var a: Int\n func foo(_ a : Int) { var b = a * 2 }") == 0);
    }

    suite("Test Parser syntax/semantics - Implicit conversions") {
        set_print_errors(true);
        test(prg("var a : Int = 2.0") == 0);
        test(prg("var a : Double = 2") == 0);
        test(prg("var a\n a = 1 \n let b: Double = a") == 0);
        test(prg("var a\n a = 1.0 \n let b: Int = a") == 0);
    }

    suite("Test Parser Example - factorial iterative") {
        test(prg_file("test/examples/factorial_iter.swift") == 0);
    }
    suite("Test Parser Example - factorial recursive") {
        test(prg_file("test/examples/factorial_recursive.swift") == 0);
    }
    suite("Test Parser Example - built-in functions") {
        test(prg_file("test/examples/builtin_functions.swift") == 0);
    }

    return 0;
}
