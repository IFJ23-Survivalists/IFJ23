/** @file string.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/10/2023
 * @brief Implementation of the module string.h.
 */

#include "string.h"
#include <string.h>
#include <stdlib.h>

void string_init(String *str) {
    if (!str) return;

    str->data = NULL;
    str->length = 0;
    str->capacity = 0;
}

void string_free(String *str) {
    if (str && str->data) {
        free(str->data);
        str->data = NULL;
        str->length = 0;
        str->capacity = 0;
    }
}

void string_clear(String *str) {
    if (string_len(str)) {
        str->length = 0;
        str->data[0] = '\0';
    }
}

void string_reserve(String *str, size_t capacity) {
    if (capacity < str->capacity) {
        return;
    }

    char *new = (char *)realloc(str->data, capacity);

    if (!new) {
        set_error(Error_Internal);
        eprint("string_reserve: Out Of Memory\n");
        return;
    }

    str->data = new;
    str->capacity = capacity;
}

size_t string_len(String *str) {
    return str->length;
}

void string_push(String *str, char ch) {
    string_reserve(str, str->length + 2);

    if (got_error()) {
        return;
    }

    str->data[str->length++] = ch;
    str->data[str->length] = '\0';
}

String string_from_c_str(const char *str) {
    String res;

    string_init(&res);

    int len = strlen(str);

    if (!len) {
        return res;
    }

    string_reserve(&res, len + 1);

    if (!got_error()) {
        strlcpy(res.data, str, len);
    }

    return res;
}

String string_take(String *str) {
    String new = {
        .data = str->data,
        .length = str->length,
        .capacity = str->capacity
    };

    str->data = NULL;
    str->length = 0;
    str->capacity = 0;

    return new;
}

String string_clone(String *str) {
    String new;

    string_reserve(&new, str->length + 1);

    if (got_error()) {
        return new;
    }

    strlcpy(new.data, str->data, str->length);

    new.length = str->length;
    new.capacity = str->length + 1;


    return new;
}

void string_remove_ident(String *str, int ident_level) {
    String new;
    string_init(&new);

    int current_ident = 0;

    for (int i = 0; str->data[i]; i++) {
        char ch = str->data[i];

        if (ch == '\n') {
            current_ident = 0;
            string_push(&new, ch);

            if (got_error()) {
                string_free(&new);
                return;
            }

            continue;
        }

        int ident = ch == '\t' ? 4
                  : ch == ' '  ? 1
                  : 0;

        if (!ident && (current_ident < ident_level)) {
             eprintf("string_remove_ident: cannot remove ident of %d levels for the given string\n%s\n", ident_level, str->data);
             string_free(&new);
             set_error(Error_Internal);
             return;
        }

        if (current_ident < ident_level) {
            current_ident += ident;
            continue;
        }

        string_push(&new, ch);
        if (got_error()) {
            string_free(&new);
            return;
        }
    }

    string_free(str);
    *str = new;
}
