#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

// 输入程序
char *user_input;

// 用于报告错误的函数，宏类型定义太弱故切换到可接受剩余参数的函数
// 接受与 printf 相同的参数
void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // 输出 pos 个空格
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// tokenizer

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
    error_at(&token->str[0], "期待得到'%c', 实际却为'%c'", op, token->str[0]);
  token = token->next;
}

// 当下一个 token 是数值时，读取下一个 token 并返回该数值。
// 否则报告错误。
int expect_number()
{
  if (token->kind != TK_NUM)
    error_at(&token->str[0], "期待得到数字, 实际却为'%c'", token->str[0]);
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

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')')
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
    error_at(p, "无法识别的 token: '%c'", *p);
  }
  new_token(TK_EOF, cur, p);
  return head.next;
}

// ast

typedef enum
{
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

struct Node
{
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *mul();
Node *primary();
Node *expr();

// expr = mul ("+" mul | "-" mul)*
Node *expr()
{
  Node *node = mul();
  for (;;)
  {
    if (consume('+'))
      node = new_node(ND_ADD, node, mul());
    else if (consume('-'))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = primary ("*" primary | "/" primary)*
Node *mul()
{
  Node *node = primary();
  for (;;)
  {
    if (consume('*'))
      node = new_node(ND_MUL, node, primary());
    else if (consume('/'))
      node = new_node(ND_DIV, node, primary());
    else
      return node;
  }
}

// primary = num | "(" expr ")"
Node *primary()
{
  if (consume('('))
  {
    Node *node = expr();
    expect(')');
    return node;
  }
  return new_node_num(expect_number());
}

// genasm

void gen(Node *node)
{
  if (node->kind == ND_NUM)
  {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind)
  {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    // cqo: 把 RAX 符号扩展为 128 位并 store 到 RDX:RAX
    printf("  idiv rdi\n");
    break;
  case ND_NUM:
    error_at(&token->str[0], "不应该生成数字节点");
  }

  printf("  push rax\n");
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "参数错误\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(user_input);
  Node *node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  gen(node);

  // 整个式子的值应该留在栈顶，所以
  // 把它加载到 RAX 作为函数的返回值
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
