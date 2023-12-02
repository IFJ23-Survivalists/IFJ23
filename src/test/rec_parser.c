/**
 * @file test/rec_parser.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 29/11/2023
 * @brief Tester for parser.
 */

#include "test.h"
#include "../parser.h"
#include "../scanner.h"

int prg(const char* source_code) {
    scanner_init_str(source_code);
    MASSERT(!got_error(), "");
    scanner_reset_to_beginning();
    MASSERT(!got_error(), "");
    parser_init();
    MASSERT(!got_error(), "");

    parser_begin();
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

int main() {
    atexit(summary);

    suite("Test - Let statements - correct") {
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
    suite("Test - Let statements - errors") {
        set_print_errors(false);
        test(prg("let a : Int = \"fuj fuj\"")== 7);
        test(prg("let a = 0\nlet a = 2") == 3);
        test(prg("let a = 0\nlet a = \":)\"") == 3);
        test(prg("let a = 0\n a = 1") == 3);
        test(prg("let a = nil") == 8);
        test(prg("let a : Int = nil") == 7);
        test(prg("let a : Int = 0.0") == 7);
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
    suite("Test - Var statements - correct") {
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
    suite("Test - Var statements - errors") {
        set_print_errors(false);
        test(prg("var a : Int = \"fuj fuj\"")== 7);
        test(prg("var a = 0\nlet a = 2") == 3);
        test(prg("var a = 0\nlet a = \":)\"") == 3);
        test(prg("var a = 0\n a = nil") == 7);
        test(prg("var a = nil") == 8);
        test(prg("var a\na = nil") == 8);
        test(prg("var a : Int = nil") == 7);
        test(prg("var a : Int = 0.0") == 7);
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
    suite("Test - Assign statements - correct") {}
    suite("Test - Assign statements - errors") {}
    suite("Test - While statements - correct") {}
    suite("Test - While statements - errors") {}
    suite("Test - Func statements - correct") {}
    suite("Test - Func statements - errors") {}
    suite("Test - Scope") {}
    suite("Test - Use of uninitialized variables") {}
    suite("Text - Example - factorial") {}

    return 0;
}
