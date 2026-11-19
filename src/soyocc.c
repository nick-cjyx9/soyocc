#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// 用于报告错误的宏
// 接受与 printf 相同的参数
#define error(fmt, ...)                              \
  do                                                 \
  {                                                  \
    fprintf(stderr, fmt __VA_OPT__(, ) __VA_ARGS__); \
    fprintf(stderr, "\n");                           \
    exit(1);                                         \
  } while (0)

// token 的种类
typedef enum
{
  TK_RESERVED, // 符号
  TK_NUM,      // 整数
  TK_EOF,      // 输入结束
} TokenKind;

typedef struct Token Token;
// 解决 Token 结构体中 Token *next 的前向声明问题
// https://zhuanlan.zhihu.com/p/675837704

struct Token
{
  TokenKind kind; // token 的类型
  Token *next;    // 下一个输入的 token
  int val;        // kind 为 TK_NUM 时，该数值
  char *str;      // token 字符串
};

// 当前正在处理的 token
Token *token;

// 当下一个 token 是所期望的符号时，读取下一个 token 并返回真。
// 否则返回假。
bool consume(char op)
{
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

// 当下一个 token 是所期望的符号时，读取下一个 token。
// 否则报告错误。
void expect(char op)
{
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error("期待得到'%c', 实际却为'%c'", op, token->str[0]);
  token = token->next;
}

// 当下一个 token 是数值时，读取下一个 token 并返回该数值。
// 否则报告错误。
int expect_number()
{
  if (token->kind != TK_NUM)
    error("期待得到数字, 实际却为'%c'", token->str[0]);
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof()
{
  return token->kind == TK_EOF;
}

// 创建新的 token 并连接到 cur 上
Token *new_token(TokenKind kind, Token *cur, char *str)
{
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// 分词
Token *tokenize(char *p)
{
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p)
  {
    if (isspace(*p))
    {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-')
    {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p))
    {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      // strtol 会将 p 移动到未处理的第一个字符上，所以不需要手动移动 p
      continue;
    }
    error("无法识别的 token: '%c'", *p);
  }
  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    error("参数错误");
    return 1;
  }

  token = tokenize(argv[1]);

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  printf("  mov rax, %d\n", expect_number());

  while (!at_eof())
  {
    if (consume('+'))
    {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}
