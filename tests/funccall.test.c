#include "test.h"
#include <stdlib.h>

int main() {
  system("echo 'int sixseven(int six, int seven)"
         "{if (six==6&&seven==7) return 67; else return 0;}' > "
         "temp_0.c");
  system("cc -c temp_0.c -o temp_0.o");
  system(SOYOCC_BIN " \"return sixseven(6, 7);\" > temp_1.s");
  system("cc temp_0.o temp_1.s -o temp");
  int ret = WEXITSTATUS(system("./temp"));
  assert(ret == 67, "Expected return value 67, but got %d", ret);
  system("rm -f temp temp_0.o temp_0.c temp_1.s");
  return EXIT_CODE;
}
