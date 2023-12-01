/**
 * @file test/rec_parser.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 29/11/2023
 * @brief Tester for rec_parser.h
 */

#include "../symstack.h"
#include "test.h"

int main() {
    atexit(summary);

    test(1);

    return 0;
}
