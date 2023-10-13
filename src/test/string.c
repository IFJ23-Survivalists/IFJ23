/**
 * @file test/string.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 13/10/2023
 * @brief Tester for string.h
 */

#include <string.h>
#include "../scanner.h"
#include "test.h"

int main() {
    atexit(summary);

    String str;

    suite("Test string_init") {
        string_init(&str);
        test(!str.length);
        test(!str.capacity);
        test(!str.data);
    }

    suite("Test string_len") {
        test(!string_len(&str));
        string_push(&str, 'h');
        test(string_len(&str) == 1);
        string_push(&str, '2');
        string_push(&str, '3');
        test(string_len(&str) == 3);
    }

    suite("Test string_clear") {
        string_clear(&str);
        test(!str.length);
        test(str.capacity);
        test(str.data);

        String empty;
        string_init(&empty);
        string_clear(&empty);
        test(!empty.length);
        test(!empty.capacity);
        test(!empty.data);
    }

    suite("Test string_concat_c_str") {
        test(!strcmp(str.data, ""));
        string_concat_c_str(&str, "h");
        test(!strcmp(str.data, "h"));
        string_concat_c_str(&str, "");
        test(!strcmp(str.data, "h"));
        string_concat_c_str(&str, "23|");
        test(!strcmp(str.data, "h23|"));
    }

    suite("Test string_reserve") {
        test(str.capacity == 5 && str.length == 4);
        string_reserve(&str, 4);
        /// cannot reserve less than the number of data in string
        test(got_error());
        set_error(Error_None);

        string_reserve(&str, 10);
        test(str.capacity == 10);
        test(str.length == 4);
    }

    suite("Test string_push") {
        test(!strcmp(str.data, "h23|"));

        string_push(&str, '_');
        test(!strcmp(str.data, "h23|_"));

        string_push(&str, '_');
        string_push(&str, '%');
        test(!strcmp(str.data, "h23|__%"));
    }

    suite("Test string_from_c_str") {
        String str2 = string_from_c_str("Hello world");

        test(str2.length == 11);
        test(str2.capacity == 12);
        test(!strcmp(str2.data, "Hello world"));
        string_free(&str2);
    }

    suite("Test string_take") {
        String str2 = string_take(&str);

        test(!str.length);
        test(!str.capacity);
        test(!str.data);
        test(str2.length == 7);
        test(str2.capacity == 10);
        test(!strcmp(str2.data, "h23|__%"));

        // Take back the data
        str = string_take(&str2);
        test(!str2.length);
        test(!str2.capacity);
        test(!str2.data);
        test(str.length == 7);
        test(str.capacity == 10);
        test(!strcmp(str.data, "h23|__%"));
    }

    suite("Test string_clone") {
        String cloned = string_clone(&str);

        // the str capacity is 10 but only need 8 in the new one
        test(cloned.capacity != str.capacity);
        // the cloned data address must not be the same as the original data address
        test(cloned.data != str.data);
        test(cloned.length == str.length);
        test(cloned.capacity == cloned.length + 1);

        string_free(&cloned);
    }

    suite("Test string_remove_ident") {
        char *input = "\tHello\n     World\n \tOnce\n\t Again";
        char *expected = "Hello\n World\nOnce\n Again";

        string_clear(&str);
        string_concat_c_str(&str, input);

        string_remove_ident(&str, 5);
        test(got_error());
        test(!strcmp(str.data, input));
        set_error(Error_None);

        string_remove_ident(&str, 4);
        test(!strcmp(str.data, expected));
    }

    suite("Test string_free") {
        string_free(&str);
        test(!str.length);
        test(!str.capacity);
        test(!str.data);
    }

    return 0;
}
