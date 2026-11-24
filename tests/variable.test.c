#include "test.h"
#include <stdlib.h>
#include <sys/wait.h>

int main() {
  system(SOYOCC_BIN
         " \"foo=12;bar=3;baz=5;return (foo+bar)/baz*(foo==foo);\" > temp.s");
  system("cc temp.s -o temp");
  // https://chatgpt.com/share/6afe66cc-bebc-83ea-af56-75c2599c8270
  int ret = WEXITSTATUS(system("./temp"));
  assert(ret == 3, "Expected return value 3, but got %d", ret);
  system("rm -f temp.s temp");
  return EXIT_CODE;
}
