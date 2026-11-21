#include "test.h"
#include <stdlib.h>
#include <sys/wait.h>

int main() {
  system(SOYOCC_BIN " 67 > temp.s");
  system("cc temp.s -o temp");
  // https://chatgpt.com/share/6afe66cc-bebc-83ea-af56-75c2599c8270
  int ret = WEXITSTATUS(system("./temp"));
  assert(ret == 67, "Expected return value 67, but got %d", ret);
  system("rm -f temp.s temp");
  return EXIT_CODE;
}
