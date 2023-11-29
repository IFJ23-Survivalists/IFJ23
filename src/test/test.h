/**
 * @file test/test.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 25/09/2023
 * @brief Simple testsuit that I got from
 * https://github.com/marcizhu/ChaCha20/blob/master/tests.c
 */

#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>

#define suite(...)                                                             \
  if (tst > 0 && prev == err)                                                  \
    printf("\r \x1b[32m✓\x1b[0m \n");                                          \
  if (printf("   \x1b[1m" __VA_ARGS__ "\x1b[0m"), prev = err, (once = 0), 1)
#define test(...)                                                              \
  do {                                                                         \
    (++tst, err += !(ok = !!(__VA_ARGS__)));                                   \
    if (!ok) {                                                                 \
      if (!once) {                                                             \
        printf("\r \x1b[31m✗\x1b[0m \n");                                      \
        once = 1;                                                              \
      }                                                                        \
      printf("   \x1b[31m✗\x1b[0m %s:%d → %s\n", __FILE__, __LINE__,           \
             #__VA_ARGS__);                                                    \
    }                                                                          \
  } while (0)
static unsigned tst = 0, err = 0, ok = 1, prev = 0, once = 0;
static void summary(void) {
  if (tst > 0 && prev == err)
    printf("\r \x1b[32m✓\x1b[0m \n");
  printf("\r  \n\x1b[1m\x1b[33m    %d total", tst);
  printf("\x1b[1m\x1b[32m   %d passed", tst - err);
  printf("\x1b[1m\x1b[31m   %d failed\x1b[0m\n", err);
  exit(err);
}

#endif
