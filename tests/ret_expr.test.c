#include "test.h"
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
  system(SOYOCC_BIN " \"(7 + 8) * 9 / 3\" > temp.s");
  system("cc temp.s -o temp");
  int ret = WEXITSTATUS(system("./temp"));
  tassert(ret == (7 + 8) * 9 / 3); // 测试加减乘除以及括号表达式的返回值
  system("rm -f temp.s temp");

  return EXIT_CODE;
}
