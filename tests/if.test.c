#include "test.h"
#include <stdlib.h>
#include <sys/wait.h>

int main() {
  system(SOYOCC_BIN " \"if(3>2) return 1; else return 0;\" > temp.s");
  system("cc temp.s -o temp");
  int ret = WEXITSTATUS(system("./temp"));
  assert(ret == 1, "Expected return value 1, but got %d", ret);
  system("rm -f temp.s temp");
  return EXIT_CODE;
}
