#include "test.h"
#include <stdlib.h>
#include <sys/wait.h>

int main() {
  system(SOYOCC_BIN " \"(7 + 8) * 9 / 3\" > temp.s");
  system("cc temp.s -o temp");
  int ret = WEXITSTATUS(system("./temp"));
  tassert(ret == (7 + 8) * 9 / 3); // 测试加减乘除以及括号表达式的返回值
  system("rm -f temp.s temp");

  system(SOYOCC_BIN " \"-(7 + 8) * -9 / 3\" > temp.s");
  system("cc temp.s -o temp");
  ret = WEXITSTATUS(system("./temp"));
  tassert(ret == -(7 + 8) * -9 / 3); // 测试一元加减法的支持
  system("rm -f temp.s temp");

  system(SOYOCC_BIN " \"- (1 == 0) + + (1 < 0) + - (0 > 1) - + ((1 <= 0) * (0 "
                    ">= 1)) / (1 != 0)\" > temp.s");
  system("cc temp.s -o temp");
  ret = WEXITSTATUS(system("./temp"));
  tassert(ret == 0); // 测试包含比较运算符的复杂表达式
  system("rm -f temp.s temp");
  return EXIT_CODE;
}
