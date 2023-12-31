/**
 * @note Project: Implementace překladače imperativního jazyka IFJ23
 * @file string.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/10/2023
 * @brief Implementation of the module string.h.
 */

#include "string.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/// Fix for Github action
size_t strlcpy(char* __restrict dst, const char* __restrict src, size_t dsize) {
    const char* osrc = src;
    size_t nleft = dsize;

    /* Copy as many bytes as will fit. */
    if (nleft != 0) {
        while (--nleft != 0) {
            if ((*dst++ = *src++) == '\0')
                break;
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src. */
    if (nleft == 0) {
        if (dsize != 0)
            *dst = '\0'; /* NUL-terminate dst */
        while (*src++)
            ;
    }

    return (src - osrc - 1); /* count does not include NUL */
}

void string_init(String* str) {
    if (!str)
        return;

    str->data = NULL;
    str->length = 0;
    str->capacity = 0;
}

void string_free(String* str) {
    if (str && str->data) {
        free(str->data);
        str->data = NULL;
        str->length = 0;
        str->capacity = 0;
    }
}

void string_clear(String* str) {
    if (string_len(str)) {
        str->length = 0;
        str->data[0] = '\0';
    }
}

void string_reserve(String* str, size_t capacity) {
    if (capacity <= str->capacity) {
        return;
    }

    char* new = (char*)realloc(str->data, sizeof(char) * capacity);

    if (!new) {
        set_error(Error_Internal);
        eprint("string_reserve: Out Of Memory\n");
        return;
    }

    str->data = new;
    str->capacity = capacity;
}

size_t string_len(String* str) {
    return str->length;
}

void string_push(String* str, char ch) {
    string_reserve(str, str->length + 2);

    if (got_error()) {
        return;
    }

    str->data[str->length++] = ch;
    str->data[str->length] = '\0';
}

void string_concat_c_str(String* str, const char* str2) {
    size_t length = strlen(str2);

    if (!length) {
        return;
    }

    size_t new_length = str->length + length;

    if (new_length >= str->capacity) {
        string_reserve(str, new_length + 1);
        if (got_error()) {
            return;
        }
    }

    strlcpy(str->data + str->length, str2, length + 1);
    str->length = new_length;
}

String string_from_c_str(const char* str) {
    String res;

    string_init(&res);

    int len = strlen(str);

    if (!len) {
        return res;
    }

    string_reserve(&res, len + 1);

    if (!got_error()) {
        strlcpy(res.data, str, len + 1);
        res.length = len;
    }

    return res;
}

String string_from_format(const char* fmt, ...) {
    String res;
    string_init(&res);

    va_list args;
    va_start(args, fmt);

    // Get the total length of the string after insertion of args.
    size_t fmt_len = vsnprintf(NULL, 0, fmt, args);

    // Allocate enough memory for the string.
    res.data = calloc(fmt_len + 1, sizeof(char));
    if (!res.data) {
        SET_INT_ERROR(IntError_Memory, "string_from_format: Calloc failed!");
        return res;
    }
    res.capacity = fmt_len + 1;

    // Reset the args, so we can use it again.
    va_end(args);
    va_start(args, fmt);

    // Insert string in given format into `res`.
    vsprintf(res.data, fmt, args);
    res.length = fmt_len;
    MASSERT(res.length <= res.capacity, "string_from_format: Wrong string length");

    va_end(args);
    return res;
}

String string_take(String* str) {
    String new = {.data = str->data, .length = str->length, .capacity = str->capacity};

    str->data = NULL;
    str->length = 0;
    str->capacity = 0;

    return new;
}

String string_clone(String* str) {
    String new;

    string_init(&new);
    string_reserve(&new, str->length + 1);

    if (got_error()) {
        return new;
    }

    strlcpy(new.data, str->data, str->length);

    new.length = str->length;
    new.capacity = str->length + 1;

    return new;
}

void string_remove_ident(String* str, int ident_level) {
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

        int ident = ch == '\t' ? 4 : ch == ' ' ? 1 : 0;

        if (!ident && (current_ident < ident_level)) {
            eprintf("string_remove_ident: cannot remove ident of %d levels for the given string\n%s\n", ident_level,
                    str->data);
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
