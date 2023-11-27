/**
 * @file test/expr_parser.c
 * @author Matúš Moravčík, xmorav48, VUT FIT
 * @date 27/11/2023
 * @brief Tester for expr_parser.h
 */

#include "../expr_parser.h"
#include <string.h>
#include "../scanner.h"
#include "test.h"

void create_test_file(char* expressions[], size_t expressionCount) {
  FILE* fp = fopen("test/expr.swift", "w");

  for (size_t i = 0; i < expressionCount; i++) {
    fputs(expressions[i], fp);
  }

  fclose(fp);
}

int main() {
  atexit(summary);

  char* expressions[] = {
      "a + b {\n",
      "a + v * 3 {\n",
      "(a + b) * 321 - 14 {\n",
      "(a + (b - (c + (d)))) / 14 {\n",
      "-j * 2 {\n",
      "(-(-(-f))) {\n",
      "1+ !a {\n",
      "1* a! +1 {\n",
      "a ?? b - 2 {\n",
      "a && fn(name: fd, u: 732) || b - 2 {\n",
      "fn(x: 1+2, y:\"asd\", z:(-7!)) {\n",
      "fn(x: inner(y: 1+ inner2(z:45))) {\n",
  };

  size_t expr_count = sizeof(expressions) / sizeof(expressions[0]);
  create_test_file(expressions, expr_count);

  FILE* file = fopen("test/expr.swift", "r+");

  if (!file) {
    eprint("Unable to read file\n");
    return 99;
  }

  scanner_init(file);

  suite("Test valid expression") {
    for (size_t i = 0; i < expr_count; i++) {
      //   printf("\n%s\n", expressions[i]);
      test(expr_parser_begin(scanner_advance_non_whitespace()) == true);
    }
  }

  //   suite("Test string_clear") {
  //     string_clear(&str);
  //     test(!str.length);
  //     test(str.capacity);
  //     test(str.data);

  //     String empty;
  //     string_init(&empty);
  //     string_clear(&empty);
  //     test(!empty.length);
  //     test(!empty.capacity);
  //     test(!empty.data);
  //   }

  fclose(file);
  return 0;
}
