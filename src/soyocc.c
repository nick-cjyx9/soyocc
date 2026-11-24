#include "soyocc.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "参数错误\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(user_input);
  codegen(program());
  return 0;
}
