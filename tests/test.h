#ifndef TEST_H
#define TEST_H
#include <stdio.h>

// test runner run tests under root path, not relative to this file
#define SOYOCC_BIN "build/soyocc"

int EXIT_CODE = 0;

#define assert(condition, ...)                                                 \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "Assertion failed: " __VA_ARGS__);                       \
      fprintf(stderr, "\n");                                                   \
      EXIT_CODE = 1;                                                           \
    }                                                                          \
  } while (0)

#define tassert(condition)                                                     \
  assert(condition, "Expect " #condition " to be true, but fails at %s:%d",    \
         __FILE__, __LINE__);

#endif
