#include "soyocc.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *user_input;
// 当前正在处理的 token
Token *token;

// 用于报告错误的函数，宏类型定义太弱故切换到可接受剩余参数的函数
// 接受与 printf 相同的参数
void error_at(char *loc, char *fmt, ...) {
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

// 当下一个 token 是所期望的符号时，读取下一个 token 并返回真。
// 否则返回假。
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != (size_t)token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// 当下一个 token 是所期望的符号时，读取下一个 token。
// 否则报告错误。
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != (size_t)token->len ||
      memcmp(token->str, op, token->len))
    error_at(&token->str[0], "期待得到'%c', 实际却为'%c'", op, token->str[0]);
  token = token->next;
}

// 当下一个 token 是数值时，读取下一个 token 并返回该数值。
// 否则报告错误。
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(&token->str[0], "期待得到数字, 实际却为'%c'", token->str[0]);
  int val = token->val;
  token = token->next;
  return val;
}

Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return NULL;
  Token *ident = token;
  token = token->next;
  return ident;
}

bool at_eof() { return token->kind == TK_EOF; }

// 创建新的 token 并连接到 cur 上
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool startswith(char *p, char *q) { return strncmp(p, q, strlen(q)) == 0; }

// C 语言的变量名必须由字母、数字和下划线组成，且不能以数字开头
bool is_ident1(char c) { return isalpha(c) || c == '_'; }
bool is_ident2(char c) { return is_ident1(c) || isdigit(c); }

// 分词
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }
    if (startswith(p, "return") && !is_ident2(p[6])) {
      cur = new_token(TK_RESERVED, cur, p, 6);
      p += 6;
      continue;
    }
    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
        startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')' || *p == '<' || *p == '>' || *p == '=' || *p == ';') {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      // strtol 会将 p 移动到未处理的第一个字符上，所以不需要手动移动 p
      continue;
    }
    if (is_ident1(*p)) {
      char *q = p;
      while (is_ident2(*p))
        p++;
      cur = new_token(TK_IDENT, cur, q, p - q);
      continue;
    }
    error_at(p, "无法识别的 token: '%c'", *p);
  }
  new_token(TK_EOF, cur, p, 1);
  return head.next;
}
